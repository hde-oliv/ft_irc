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

Server::Server() {}

Server::Server(std::string const &password, int const &port) {
	this->password	= password;
	this->port		= port;
	this->server_fd = 0;
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
		panic("socket(Server:36)", "Failed");
	}

	err = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
					 sizeof(opt));
	if (err) {
		panic("setsockopt(Server:43)", "Failed");
	}

	err = bind(server_fd, (struct sockaddr *)&(address), sizeof(address));
	if (err < 0) {
		panic("bind(Server:48)", "Failed");
	}

	err = listen(server_fd, SOMAXCONN);
	if (err < 0) {
		panic("listen(Server:53)", "Failed");
	}
}

void Server::startServer() {
	setupSocket();

	pollfds[0].fd	  = server_fd;
	pollfds[0].events = POLLIN;
	poll_index		  = 0;

	while (true) {
		int r = poll(pollfds, poll_index + 1, -1);

		if (r < 0) {
			panic("poll(Server:67)", "Failed");
		}

		serverEventHandling();
		clientEventHandling();
	}

	close(server_fd);
}

void Server::serverEventHandling() {
	struct sockaddr_in client_address;
	int				   addrlen = sizeof(client_address);

	if (pollfds[0].revents & POLLIN) {
		int fd = accept(server_fd, (struct sockaddr *)&client_address,
						(socklen_t *)&addrlen);
		if (fd < 0) {
			panic("accept(Server:83)", "Failed");
		}

		if (poll_index < MAX_CLIENTS) {
			Client *newClient = new Client;

			newClient->setHost(inet_ntoa(client_address.sin_addr));
			newClient->setFd(fd);

			clients[fd]					   = *newClient;
			pollfds[poll_index + 1].fd	   = fd;
			pollfds[poll_index + 1].events = POLLIN;
			poll_index++;
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

			char buffer[BUFFER_SIZE];  // TODO: use client buffer, currently
									   // bugged

			ft_memset(buffer, 0, BUFFER_SIZE);

			ssize_t v = read(pollfds[i].fd, buffer, BUFFER_SIZE);

			if (v == -1) {
				panic("read(Server:117)", "Failed");
			} else if (v == 0) {
				std::cerr << "Client disconnected. Client socket: "
						  << pollfds[i].fd << std::endl;
				close(pollfds[i].fd);

				pollfds[i] = pollfds[poll_index];
				ft_memset(&(pollfds[poll_index]), 0,
						  sizeof(pollfds[poll_index]));

				poll_index--;
			} else {
				std::cout << "Client " << pollfds[i].fd
						  << " =================== Start" << std::endl;
				std::cout << buffer;
				std::cout << "Client " << pollfds[i].fd
						  << " =================== End" << std::endl;

				// TODO: Remove this later
				if (write(pollfds[i].fd, buffer, v) == -1) {
					panic("write(Server:137)", "Failed");
				}
			}
		}
	}
}
