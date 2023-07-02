#include "Server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Client.hpp"
#include "Utils.hpp"

extern bool g_online;  // TODO: Consider using a singleton, globals are ugly

Server::Server() {}

Server::Server(std::string const &password, int const &port) {
	this->password	= password;
	this->port		= port;
	this->server_fd = 0;
}

Server::~Server() {}

void Server::ejectClient(int clientFd, int reason) {
	close(clientFd);

	for (int i = 1; i < MAX_CLIENTS; i++) {
		if (pollfds[i].fd == clientFd)
			std::memset(&pollfds[i], 0, sizeof(pollfd));
	}

	clients.erase(clientFd);
	poll_index--;

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

void Server::setupSocket() {
	int opt = 1;
	int err;

	address.sin_family		= AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port		= htons(port);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd < 0) {
		panic("Server::socket", "Failed");
	}

	err = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
					 sizeof(opt));
	if (err) {
		panic("Server::setsockopt", "Failed");
	}

	err = bind(server_fd, (struct sockaddr *)&(address), sizeof(address));
	if (err < 0) {
		panic("Server::bind", "Failed");
	}

	err = listen(server_fd, SOMAXCONN);
	if (err < 0) {
		panic("Server::listen", "Failed");
	}
}

void Server::startServer() {
	setupSocket();
	std::memset(pollfds, 0, sizeof(pollfd) * MAX_CLIENTS);

	pollfds[0].fd	  = server_fd;
	pollfds[0].events = POLLIN;
	poll_index		  = 0;

	while (g_online) {
		int r = poll(pollfds, poll_index + 1, -1);

		if (r >= 0) {
			serverEventHandling();
			clientEventHandling();
		}
	}

	close(server_fd);
}

pollfd &Server::getAvailablePollFd() {
	int i = 1;

	while (i < MAX_CLIENTS) {
		if (pollfds[i].fd == 0) {
			break;
		}
		i++;
	}

	if (i == MAX_CLIENTS) {
		panic("Server::getAvailablePollFd",
			  "Server could not find an available pollfd.");
	}

	return pollfds[i];
}

void Server::serverEventHandling() {
	if (pollfds[0].revents & POLLIN) {
		newClientHandling();
	}
}

void Server::clientEventHandling() {
	for (int i = 1; i <= poll_index; i++) {
		if (pollfds[i].revents & POLLIN) {
			readFromClient(pollfds[i]);
		} else if (pollfds[i].revents & POLLOUT) {
			sendToClient(pollfds[i]);
		} else if (pollfds[i].revents & POLLERR) {
			std::cout << "POLLERR caught" << std::endl;
			ejectClient(pollfds[i].fd, -1);
		}

		// NOTE: I do not know if they are necessary
		// else if (pollfds[i].revents & POLLHUP) {
		// 	std::cout << "POLLHUP caught" << std::endl;
		// } else if (pollfds[i].revents & POLLNVAL) {
		// 	std::cout << "POLLNVAL caught" << std::endl;
		// }
	}
}

void Server::readFromClient(pollfd p) {
	Client *c = &clients[p.fd];

	char	buffer[BUFFER_SIZE];
	ssize_t bytesRead;

	std::memset(buffer, 0, BUFFER_SIZE);
	bytesRead = recv(p.fd, buffer, BUFFER_SIZE, 0);

	if (bytesRead == -1) {
		// TODO: implement ejectAllClients();
		panic("Server::recv", "Failed");
	} else if (bytesRead == 0) {
		ejectClient(p.fd, LOSTCONNECTION);
	} else {
		c->setReadData(buffer);

		// if (c->getReadData().find("\r\n") != std::string::npos) {
		// TODO: parse message and set sendData string
		// }
		c->setSendData(c->getReadData());
	}

	std::cout << "Client " << p.fd << " sent: ";
	std::cout << c->getReadData();
}

void Server::sendToClient(pollfd p) {
	Client *c = &clients[p.fd];

	if (send(p.fd, c->getSendData().c_str(), c->getSendData().size(), 0) ==
		-1) {
		panic("Server::send", "Failed");
	}
	c->resetData();
}

void Server::newClientHandling() {
	struct sockaddr_in client_address;
	int				   addrlen = sizeof(client_address);

	int fd = accept(server_fd, (struct sockaddr *)&client_address,
					(socklen_t *)&addrlen);

	if (fd < 0) {
		panic("Server::accept", "Failed");
	}

	if (poll_index < MAX_CLIENTS) {
		Client	newClient;
		pollfd &clientPollFd = getAvailablePollFd();

		newClient.setHost(inet_ntoa(client_address.sin_addr));
		newClient.setFd(fd);

		clients[fd]			= newClient;
		clientPollFd.fd		= fd;
		clientPollFd.events = POLLIN | POLLOUT | POLLERR;
		poll_index++;
		std::cout << "New connection stablished with " << newClient.getHost()
				  << " on fd " << fd << std::endl;
	} else {
		std::cerr << "Maximum number of clients reached. Connection rejected"
				  << std::endl;
		close(fd);
	}
}
