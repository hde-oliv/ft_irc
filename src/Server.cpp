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
void Server::ejectDisconnected() {
	for (std::map<int, Client>::reverse_iterator it = clients.rbegin();
		 it != clients.rend(); it--) {
		if (!(*it).second.connected) {
			std::cout << "Should have disconnected someone" << std::endl;
			ejectClient((*it).second.getFd(), QUITED);
			break;
		}
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
			ejectDisconnected();
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
				ejectClient((*it).fd, -1);
			} else if ((*it).revents & POLLHUP) {
				std::cout << "POLLHUP caught" << std::endl;
			} else if ((*it).revents & POLLNVAL) {
				std::cout << "POLLNVAL caught" << std::endl;
			}
		}
	}
}

void Server::newClientHandling() {
	struct sockaddr_in client_address;
	int				   addrlen = sizeof(client_address);

	int fd = accept(server_fd, (struct sockaddr *)&client_address,
					(socklen_t *)&addrlen);

	if (fd < 0) {
		panic("Server::accept", "Failed", P_CONTINUE);
		return;
	}

	if (pollFds.size() + 1 < MAX_CLIENTS) {
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

// NOTE: I think that when running this will produce a lot of vectors of size 1
// And in the current state of readFromClient it does not seem to break further
// development of the other commands and stuff.
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
			keepReading = false;
		}
	}
	while (c->inBuffer.find("\r\n") != std::string::npos) {
		pos = c->inBuffer.find("\r\n");
		if (pos > 0) {
			c->cmdVec.push_back(c->inBuffer.substr(0, pos + 2));
			c->inBuffer.erase(0, pos + 2);
		}
	}
}

void Server::readFromClient(pollfd p) {
	Client *c = &clients[p.fd];

	recvLoop(p);

	for (std::vector<std::string>::iterator it = c->cmdVec.begin();
		 it < c->cmdVec.end(); it++) {
		std::cout << "Client " << p.fd << " sent: ";
		std::cout << RED << (*it) << RESET;
		std::string response = executeClientMessage(p, (*it));
		if (response == "KICK CLIENT") {
			c->connected = false;
			// ejectClient(p.fd, KICKED);
		}
		c->setSendData(response);
	}
	c->cmdVec.clear();
}

void Server::sendToClient(pollfd p) {
	Client *c = &clients[p.fd];
	int		r;

	// send might send the message partially, needs handling other than OK and
	// fail (controll how manyu bytes were actually sent).
	// it would be best the erase characters thar were correctly sent instead
	// of erasing the entire string
	//
	// NOTE: Check if the changes here and on resetSendData fix that ^

	if (c->getSendData().size()) {
		r = send(p.fd, c->getSendData().c_str(), c->getSendData().size(), 0);
		if (r == -1) {
			panic("Server::send", "Failed", P_CONTINUE);
		} else if (r > 0) {
			c->resetSendData(r);
		}
	}
}

void Server::ejectClient(int clientFd, int reason) {
	for (std::vector<pollfd>::iterator it = pollFds.begin(); it < pollFds.end();
		 it++) {
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

// Command section

std::string Server::executeClientMessage(pollfd p, std::string msg) {
	std::vector<std::string> cmdList{
		"NICK",		"PASS",	  "USER",	"QUIT",	   "OPER",	 "JOIN",  "PING",
		"PART",		"MODE",	  "NAMES",	"LIST",	   "INVITE", "KICK",  "VERSION",
		"STATS",	"LINKS",  "TIME",	"CONNECT", "TRACE",	 "ADMIN", "INFO",
		"PRIVMSG",	"NOTICE", "WHO",	"WHOIS",   "WHOWAS", "KILL",  "PONG",
		"ERROR",	"AWAY",	  "REHASH", "RESTART", "SUMMON", "USERS", "WALLOPS",
		"USERHOST", "ISON",	  "SQUIT",	"SERVER"
	};
	std::vector<std::string>::iterator it;

	Client	   *c  = &clients[p.fd];
	Command		cm = stringToCommand(msg);
	std::string response;

	std::cout << YELLOW << msg << RESET;
	std::cout << BLUE << cm.cmd << RESET << std::endl;
	std::cout << YELLOW << c->getRegistration() << RESET << std::endl;

	it = find(cmdList.begin(), cmdList.end(), cm.cmd);

	ssize_t index;

	if (it != cmdList.end()) {
		index = it - cmdList.begin();
	} else {
		index = -1;
	}

	switch (index) {
		case 0:
			response = nick(p, cm);
		case 1:
			response = pass(p, cm);
		case 2:
			response = user(p, cm);
		case -1:
		default:
			response = unknowncommand(p, cm.cmd);
	}

	if (c->getRegistration() == (PASS_FLAG | USER_FLAG | NICK_FLAG) &&
		!c->getWelcome()) {
		c->setWelcome(true);
		response = welcome(p);
	}

	return response;
}

void Server::broadcastMessage(std::string message) {
	std::map<int, Client>::iterator it = clients.begin();

	for (; it != clients.end(); it++) {
		(it->second).setSendData(message);
	}
}

void Server::disconnectHandling() {
	std::map<int, Client>::iterator it = clients.begin();

	for (; it != clients.end(); it++) {
		if ((it->second).getToDisconnect()) {
			ejectClient(it->first, QUITED);
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

		broadcastMessage(ss.str());
	}
	c->setToDisconnect(true);
}
