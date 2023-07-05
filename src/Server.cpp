#include "Server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
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
			// ejections should happen after both!
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
	// for (std::vector<pollfd>::iterator it = pollFds.begin() + 1;
	// 	 it < pollFds.end(); it++) {
	// 	if ((*it).revents & POLLIN) {
	// 		readFromClient((*it));
	// 	} else if ((*it).revents & POLLOUT) {
	// 		sendToClient((*it));
	// 	} else if ((*it).revents & POLLERR) {
	// 		std::cout << "POLLERR caught" << std::endl;
	// 		ejectClient((*it).fd, -1);
	// 	} else if ((*it).revents & POLLHUP) {
	// 		std::cout << "POLLHUP caught" << std::endl;
	// 	} else if ((*it).revents & POLLNVAL) {
	// 		std::cout << "POLLNVAL caught" << std::endl;
	// 	}
	// }
	// for (std::vector<pollfd>::size_type i = 1; i <= pollFds.size(); i++) {
	// 	if (pollFds[i].revents & POLLIN) {
	// 		readFromClient(pollFds[i]);
	// 	} else if (pollFds[i].revents & POLLOUT) {
	// 		sendToClient(pollFds[i]);
	// 	} else if (pollFds[i].revents & POLLERR) {
	// 		std::cout << "POLLERR caught" << std::endl;
	// 		ejectClient(pollFds[i].fd, -1);
	// 	} else if (pollFds[i].revents & POLLHUP) {
	// 		std::cout << "POLLHUP caught" << std::endl;
	// 	} else if (pollFds[i].revents & POLLNVAL) {
	// 		std::cout << "POLLNVAL caught" << std::endl;
	// 	}
	// }
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
void recvLoop(pollfd p) {
	char		buffer[BUFFER_SIZE];
	ssize_t		bytesRead;
	bool		keepReading = true;
	std::string strBuff		= "";
	std::string clientBuff	= "";  // this will be moved into client struct
	std::vector<std::string> cmds;
	size_t					 pos;

	while (keepReading) {
		std::memset(buffer, 0, BUFFER_SIZE);
		bytesRead = recv(p.fd, buffer, BUFFER_SIZE,
						 MSG_PEEK);	 // MSG_PEEK keeps data in the fd
		if (bytesRead > 0) {
			strBuff.append(buffer);
			if (bytesRead < BUFFER_SIZE) {
				keepReading = false;
			}
		} else {
			keepReading = false;
		}
	}
	clientBuff.append(strBuff);
	strBuff.erase();
	keepReading = true;
	while (keepReading) {
		// (pos = clientBuff.find("\n\r")) != std::string::npos
		if (clientBuff.length() == 0) break;
		pos = clientBuff.find("\r\n");
		if (pos > 0) {
			cmds.push_back(clientBuff.substr(0, pos));
			clientBuff.erase(0, pos + 2);
		} else {
			cmds.push_back(clientBuff);
			clientBuff.erase();
		}
	}
}

void Server::readFromClient(pollfd p) {
	Client *c = &clients[p.fd];

	char	buffer[BUFFER_SIZE];
	ssize_t bytesRead;

	recvLoop(p);

	std::memset(buffer, 0, BUFFER_SIZE);
	bytesRead = recv(p.fd, buffer, BUFFER_SIZE, 0);

	if (bytesRead == -1) {
		// TODO: implement ejectAllClients();
		ejectClient(p.fd, LOSTCONNECTION);
		panic("Server::recv", "Failed", P_CONTINUE);
		return;
	} else if (bytesRead == 0) {
		ejectClient(p.fd, LOSTCONNECTION);
		std::cout << "This is where there would be a disconnect event"
				  << std::endl;
	} else {
		c->setReadData(buffer);

		if (c->getReadData().find("\r\n") != std::string::npos) {
			std::cout << "Client " << p.fd << " sent: ";
			std::cout << RED << c->getReadData() << RESET;

			std::string response = executeClientMessage(p, c->getReadData());

			// TODO: Remove this later
			if (response == "KICK CLIENT") {
				ejectClient(p.fd, -1);
			}

			c->setSendData(response);
		}
	}
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
	std::string registration[] = { "PASS", "USER", "NICK" };

	Client *c  = &clients[p.fd];
	Command cm = stringToCommand(msg);

	std::cout << YELLOW << msg << RESET;
	std::cout << BLUE << cm.cmd << RESET << std::endl;
	std::cout << YELLOW << c->getRegistration() << RESET << std::endl;

	// TODO: Refactor
	if (c->getRegistration() != (PASS_FLAG | USER_FLAG | NICK_FLAG)) {
		std::string response;

		if (cm.cmd == "PASS")
			response = pass(p, cm);
		else if (cm.cmd == "USER")
			response = user(p, cm);
		else if (cm.cmd == "NICK")
			response = nick(p, cm);

		if (c->getRegistration() == (PASS_FLAG | USER_FLAG | NICK_FLAG) &&
			!c->getWelcome()) {
			c->setWelcome(true);
			response = welcome(p);
		}
		return response;
	}

	return "KICK CLIENT";
}
