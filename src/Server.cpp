#include "Server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Utils.hpp"

Server::Server() {}

Server::Server(std::string const &password, int const &port) {
	this->password	= password;
	this->port		= port;
	this->server_fd = 0;
	this->clients	= 0;
}

Server::~Server() {}

void Server::setupSocket() {
	int opt = 1;
	int err;

	this->address.sin_family	  = AF_INET;
	this->address.sin_addr.s_addr = INADDR_ANY;
	this->address.sin_port		  = htons(this->port);

	this->server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (this->server_fd < 0) {
		panic("socket(Server:36)", "Failed");
	}

	err = setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
					 &opt, sizeof(opt));
	if (err) {
		panic("setsockopt(Server:43)", "Failed");
	}

	err = bind(this->server_fd, (struct sockaddr *)&(this->address),
			   sizeof(this->address));
	if (err < 0) {
		panic("bind(Server:48)", "Failed");
	}

	err = listen(this->server_fd, SOMAXCONN);
	if (err < 0) {
		panic("listen(Server:53)", "Failed");
	}
}

void Server::startServer() {
	setupSocket();

	this->fds[0].fd		= this->server_fd;
	this->fds[0].events = POLLIN;

	while (true) {
		int r = poll(this->fds, this->clients + 1, -1);

		if (r < 0) {
			panic("poll(Server:67)", "Failed");
		}

		serverEventHandling();
		clientEventHandling();
	}

	close(this->server_fd);
}

void Server::serverEventHandling() {
	struct sockaddr_in client_address;
	int				   addrlen = sizeof(client_address);

	if (this->fds[0].revents & POLLIN) {
		// New client
		int fd = accept(this->server_fd, (struct sockaddr *)&client_address,
						(socklen_t *)&addrlen);
		if (fd < 0) {
			panic("poll(Server:86)", "Failed");
		}

		std::cout << "New connection accepted. Client address: "
				  << inet_ntoa(client_address.sin_addr) << std::endl;

		if (clients < MAX_CLIENTS) {
			this->fds[clients + 1].fd	  = fd;
			this->fds[clients + 1].events = POLLIN;
			this->clients++;
		} else {
			std::cout
				<< "Maximum number of clients reached. Connection rejected"
				<< std::endl;
			close(fd);
		}
	}
}

void Server::clientEventHandling() {
	for (int i = 1; i <= this->clients; i++) {
		if (this->fds[i].revents & POLLIN) {
			char buffer[BUFFER_SIZE];

			ft_memset(buffer, 0, sizeof(buffer));  // TODO: memset not allowed

			ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer));

			if (bytes_read == -1) {
				std::cout << "read failed" << std::endl;
				exit(EXIT_FAILURE);
			} else if (bytes_read == 0) {
				std::cout << "Client disconnected. Client socket: " << fds[i].fd
						  << std::endl;
				close(this->fds[i].fd);

				this->fds[i] = this->fds[clients];
				ft_memset(&(this->fds[clients]), 0, sizeof(this->fds[clients]));
				this->clients--;
			} else {
				std::cout << "Client " << this->fds[i].fd
						  << " =================== Start" << std::endl;
				std::cout << buffer;
				std::cout << "Client " << this->fds[i].fd
						  << " =================== End" << std::endl;

				// Echo the data back to the client
				if (write(this->fds[i].fd, buffer, bytes_read) == -1) {
					std::cout << "write failed" << std::endl;
					exit(EXIT_FAILURE);
				}
			}
		}
	}
}
