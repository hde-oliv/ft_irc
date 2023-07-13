#include "Server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include "Client.hpp"
#include "Utils.hpp"

extern bool g_online;  // TODO: Consider using a singleton, globals are ugly

Server::Server() {}

Server::Server(std::string const &password, int const &port) {
	this->password		   = password;
	this->port			   = port;
	this->server_fd		   = 0;
	this->creationDatetime = getDatetime();
}

Server::~Server() {}

void Server::setupSocket() {
	int opt = 1;
	int err;

	address.sin_family		= AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port		= htons(port);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd < 0) {
		panic("Server::socket", "Failed", P_EXIT);
	}

	err = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
					 sizeof(opt));
	if (err) {
		panic("Server::setsockopt", "Failed", P_EXIT);
	}

	err = bind(server_fd, (struct sockaddr *)&(address), sizeof(address));
	if (err < 0) {
		panic("Server::bind", "Failed", P_EXIT);
	}

	err = listen(server_fd, SOMAXCONN);
	if (err < 0) {
		panic("Server::listen", "Failed", P_EXIT);
	}
}

void Server::startServer() {
	setupSocket();
	int		r;
	pollfd *cVectorPollfds;
	nfds_t	pollSize;

	struct pollfd server;
	server.fd	   = server_fd;
	server.events  = POLLIN;
	server.revents = 0;
	pollFds.push_back(server);
	while (g_online) {
		cVectorPollfds = pollFds.data();
		pollSize	   = static_cast<nfds_t>(pollFds.size());
		r			   = poll(cVectorPollfds, pollSize, -1);
		if (r >= 0) {
			serverEventHandling();
			clientEventHandling();
			disconnectHandling();
		}
	}
	close(server_fd);
}

void Server::serverEventHandling() {
	if (pollFds[0].revents & POLLIN) {
		newClientHandling();
	}
}

void Server::clientEventHandling() {
	std::vector<pollfd>::iterator it = pollFds.begin();
	if (pollFds.size() > 1) {
		while (++it < pollFds.end()) {
			if ((*it).revents & POLLIN) {
				readFromClient((*it));
			} else if ((*it).revents & POLLOUT) {
				sendToClient((*it));
			} else if ((*it).revents & POLLERR) {
				std::cout << "POLLERR caught" << std::endl;
				unexpectedDisconnectHandling(*it);
			} else if ((*it).revents & POLLHUP) {
				std::cout << "POLLHUP caught" << std::endl;
			} else if ((*it).revents & POLLNVAL) {
				std::cout << "POLLNVAL caught" << std::endl;
			}
		}
	}
}

std::map<std::string, Channel>::iterator Server::getChannelByName(
	std::string channelName) {
	std::string upperInput = toIrcUpperCase(channelName);

	std::map<std::string, Channel>::iterator it = channels.begin();
	while (it != channels.end()) {
		std::string upperChannel = toIrcUpperCase((*it).first);
		if (upperInput == upperChannel) {
			return it;
		}
		it++;
	}
	return channels.end();
};

void Server::newClientHandling() {
	struct sockaddr_in client_address;
	int				   addrlen = sizeof(client_address);

	int fd = accept(server_fd, (struct sockaddr *)&client_address,
					(socklen_t *)&addrlen);

	if (fd < 0) {
		panic("Server::accept", "Failed", P_CONTINUE);
		return;
	}

	if (pollFds.size() < MAX_CLIENTS + 1) {
		Client newClient;
		pollfd newPollFd;

		newClient.setHostname(inet_ntoa(client_address.sin_addr));
		newClient.setFd(fd);
		clients[fd] = newClient;

		newPollFd.fd	  = fd;
		newPollFd.events  = POLLIN | POLLOUT | POLLERR | POLLHUP | POLLNVAL;
		newPollFd.revents = 0;
		pollFds.push_back(newPollFd);
		std::cout << "New connection stablished with "
				  << newClient.getHostname() << " on fd " << fd << std::endl;
	} else {
		std::cerr << "Maximum number of clients reached. Connection rejected"
				  << std::endl;
		close(fd);
	}
}

// This functions takes in a pollfd p, finds the Client in Server::clients that
// is associated with p, than loads the Client->cmdVec with strings containg the
// raw commands received by the server. cmvVec acts like a quee of messages
// received and ready to be processed
void Server::recvLoop(pollfd p) {
	char	buffer[BUFFER_SIZE];
	ssize_t bytesRead;
	size_t	pos;
	bool	keepReading = true;
	Client *c			= &clients[p.fd];

	while (keepReading) {
		std::memset(buffer, 0, BUFFER_SIZE);
		bytesRead = recv(p.fd, buffer, BUFFER_SIZE, 0);
		if (bytesRead > 0) {
			c->inBuffer.append(buffer);
			if (bytesRead < BUFFER_SIZE) {
				keepReading = false;
			}
		} else {
			unexpectedDisconnectHandling(p);
			keepReading = false;
		}
	}
	while (c->inBuffer.find("\r\n") != std::string::npos) {
		pos = c->inBuffer.find("\r\n");
		if (pos > 0) {
			c->cmdVec.push_back(c->inBuffer.substr(0, pos));
			c->inBuffer.erase(0, pos + 2);
			/*
			last.erase(std::remove(last.begin(), last.end(), '\r'), last.end());
			std::string &last = c->cmdVec.back();
			last.erase(std::remove(last.begin(), last.end(), '\n'), last.end());
			*/
		}
	}
}

void Server::readFromClient(pollfd p) {
	Client *c = &clients[p.fd];

	recvLoop(p);

	std::vector<std::string>::iterator it = c->cmdVec.begin();

	for (; it < c->cmdVec.end(); it++) {
		// DEBUG
		std::cout << "Client " << c->getNickname() << " " << p.fd << " sent: ";
		std::cout << RED << (*it) << RESET << std::endl;
		executeClientMessage(p, (*it));
	}
	c->cmdVec.clear();
}

void Server::sendToClient(pollfd p) {
	Client *c = &clients[p.fd];
	int		r;

	if (c->getSendData().size()) {
		r = send(p.fd, c->getSendData().c_str(), c->getSendData().size(), 0);

		// DEBUG
		std::cout << CYAN << c->getSendData() << RESET;

		if (r == -1) {
			panic("Server::send", "Failed", P_CONTINUE);
		} else if (r > 0) {
			c->resetSendData(r);
		}
	}
}

void Server::ejectClient(int clientFd, int reason) {
	std::vector<pollfd>::iterator it = pollFds.begin();

	for (; it < pollFds.end(); it++) {
		if ((*it).fd == clientFd) {
			close(clientFd);
			std::memset(&(*it), 0, sizeof(pollfd));
			pollFds.erase(it);
			break;
		}
	}
	clients.erase(clientFd);

	switch (reason) {
		case LOSTCONNECTION:
			std::cout << "Client connection lost. (fd : " << clientFd << ")"
					  << std::endl;
			break;
		case QUITED:
			std::cout << "Client left. (fd : " << clientFd << ")" << std::endl;
			break;
		case KICKED:
			std::cout << "Client successfully kicked. (fd : " << clientFd << ")"
					  << std::endl;
			break;
		default:
			std::cout << "Client successfully ejected. (fd : " << clientFd
					  << ")" << std::endl;
	}
}

void Server::executeClientMessage(pollfd p, std::string msg) {
	Client	   *c  = &clients[p.fd];
	Command		cm = messageToCommand(msg);
	std::string response;

	// DEBUG
	std::cout << BLUE << cm.cmd << RESET << std::endl;
	std::cout << YELLOW << c->getRegistration() << RESET << std::endl;

	if (cm.cmd == "NICK") {
		nick(p, cm);
	} else if (cm.cmd == "PASS") {
		pass(p, cm);
	} else if (cm.cmd == "USER") {
		user(p, cm);
	} else if (cm.cmd == "OPER") {
		oper(p, cm);
	} else if (cm.cmd == "JOIN") {
		join(p, cm);
	} else if (cm.cmd == "QUIT") {
		quit(p, cm);
	} else if (cm.cmd == "PING") {
		ping(p, cm);
	} else if (cm.cmd == "MODE") {
		mode(p, cm);
	} else if (cm.cmd == "PRIVMSG") {
		privmsg(p, cm);
	} else if (cm.cmd == "WHO") {
		who(p, cm);	 // TODO
	} else if (cm.cmd == "WHOIS") {
		whois(p, cm);  // TODO
	} else if (cm.cmd == "WHOWAS") {
		whowas(p, cm);	// TODO
	} else {
		c->setSendData(unknowncommand(p, cm.cmd));
	}

	if (c->getRegistration() == (PASS_FLAG | USER_FLAG | NICK_FLAG) &&
		!c->getWelcome()) {
		c->setWelcome(true);
		c->setSendData(welcome(p));
		c->setSendData(motd(p));
	}
}

void Server::broadcastMessage(pollfd sender, std::string message) {
	std::map<int, Client>::iterator it = clients.begin();

	for (; it != clients.end(); it++) {
		if ((it->first) != sender.fd) {
			(it->second).setSendData(message);
		}
	}
}

void Server::disconnectHandling() {
	std::map<int, Client>::reverse_iterator it = clients.rbegin();

	for (; it != clients.rend(); it++) {
		if ((it->second).getToDisconnect()) {
			std::cout << "Should have disconnected someone." << std::endl;
			ejectClient(it->first, QUITED);
			break;
		}
	}
}

void Server::unexpectedDisconnectHandling(pollfd p) {
	Client			 *c = &clients[p.fd];
	std::stringstream ss;

	if (c->getRegistration() == (NICK_FLAG | USER_FLAG | PASS_FLAG)) {
		ss << ":" << c->getNickname();
		ss << "!" << c->getUsername();
		ss << "@" << c->getHostname();
		ss << " QUIT: Client exited unexpectedly";
		ss << "\r\n";

		broadcastMessage(p, ss.str());
	}
	c->setToDisconnect(true);
}
