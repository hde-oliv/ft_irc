#include "Server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
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
			newClientHandling();
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

void Server::newClientHandling() {
	struct sockaddr_in client_address;
	int				   addrlen = sizeof(client_address);

	if (pollfds[0].revents & POLLIN) {
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
			clientPollFd.events = POLLIN;
			poll_index++;
			std::cout << "New connection stablished with "
					  << newClient.getHost() << " on fd " << fd << std::endl;
		} else {
			std::cerr
				<< "Maximum number of clients reached. Connection rejected"
				<< std::endl;
			close(fd);
		}
	}
}

void Server::clientEventHandling() {
	for (int i = 1; i <= poll_index; i++) {
		if (pollfds[i].revents & POLLIN) {
			Client *c = &clients[pollfds[i].fd];
			(void)c;

			char buffer[BUFFER_SIZE];  // Messages overs 512 chars will not be
									   // entirely read, thus, reentering this
									   // functions in the next loop

			// REVIEW: Shouldn't it discard everything after 512 and only read
			// until a CRLF?

			std::memset(buffer, 0, BUFFER_SIZE);

			ssize_t v = read(pollfds[i].fd, buffer, BUFFER_SIZE);

			if (v == -1) {
				panic("Server::read", "Failed");
			} else if (v == 0) {
				ejectClient(pollfds[i].fd, LOSTCONNECTION);
			} else {
				std::cout << "Client " << pollfds[i].fd
						  << " =================== Start" << std::endl;
				std::cout << buffer;
				std::cout << "Client " << pollfds[i].fd
						  << " =================== End" << std::endl;

				// TODO: Remove this later
				if (write(pollfds[i].fd, buffer, v) == -1) {
					panic("Server::write", "Failed");
				}
			}
		} else if (pollfds[i].revents & POLLOUT) {
			std::cout << "POLLOUT caught" << std::endl;
		} else if (pollfds[i].revents & POLLERR) {
			std::cout << "POLLERR caught" << std::endl;
		} else if (pollfds[i].revents & POLLHUP) {
			std::cout << "POLLHUP caught" << std::endl;
		} else if (pollfds[i].revents & POLLNVAL) {
			std::cout << "POLLNVAL caught" << std::endl;
		}
	}
}
