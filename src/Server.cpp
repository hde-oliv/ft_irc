#include "Server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

Server::Server() {}

Server::Server(std::string const &password, int const &port) {
	this->password = password;
	this->port	   = port;
	server_fd	   = 0;
	clients		   = 0;
}

Server::Server(Server const &ref) {
	this->password = ref.password;
	this->port	   = ref.port;
	server_fd	   = ref.server_fd;
	clients		   = ref.clients;
}

Server::~Server() {}

Server &Server::operator=(const Server &ref) {
	this->password = ref.password;
	this->port	   = ref.port;
	server_fd	   = ref.server_fd;
	clients		   = ref.clients;

	return *this;
}

void Server::setupFD() {
	int opt = 1;
	int err;

	address.sin_family		= AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port		= htons(this->port);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd < 0) {
		std::cout << "socket failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	err = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
					 sizeof(opt));
	if (err) {
		std::cout << "setsockopt failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	err = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	if (err < 0) {
		std::cout << "bind failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	err = listen(server_fd, SOMAXCONN);
	if (err < 0) {
		std::cout << "listen failed" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Server::startServer() {
	setupFD();

	fds[0].fd	  = server_fd;
	fds[0].events = POLLIN;

	while (true) {
		int r = poll(fds, this->clients + 1, -1);

		if (r < 0) {
			std::cout << "poll failed" << std::endl;
			exit(EXIT_FAILURE);
		}

		serverEventHandling();
		clientEventHandling();
	}

	close(server_fd);
}

void Server::serverEventHandling() {
	struct sockaddr_in client_address;
	int				   addrlen = sizeof(client_address);

	if (fds[0].revents & POLLIN) {
		// New client
		int fd = accept(server_fd, (struct sockaddr *)&client_address,
						(socklen_t *)&addrlen);
		if (fd < 0) {
			std::cout << "accept failed" << std::endl;
			exit(EXIT_FAILURE);
		}

		std::cout << "New connection accepted. Client address: "
				  << inet_ntoa(client_address.sin_addr) << std::endl;

		if (clients < MAX_CLIENTS) {
			fds[clients + 1].fd		= fd;
			fds[clients + 1].events = POLLIN;
			clients++;
		} else {
			std::cout
				<< "Maximum number of clients reached. Connection rejected"
				<< std::endl;
			close(fd);
		}
	}
}

void Server::clientEventHandling() {
	for (int i = 1; i <= clients; i++) {
		if (fds[i].revents & POLLIN) {
			char buffer[BUFFER_SIZE];

			memset(buffer, 0, sizeof(buffer));	// TODO: memset not allowed

			ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer));

			if (bytes_read == -1) {
				std::cout << "read failed" << std::endl;
				exit(EXIT_FAILURE);
			} else if (bytes_read == 0) {
				std::cout << "Client disconnected. Client socket: " << fds[i].fd
						  << std::endl;
				close(fds[i].fd);

				fds[i] = fds[clients];
				memset(&fds[clients], 0,
					   sizeof(fds[clients]));  // TODO: memset not allowed
				clients--;
			} else {
				std::cout << "Client " << fds[i].fd
						  << " =================== Start" << std::endl;
				std::cout << buffer;
				std::cout << "Client " << fds[i].fd
						  << " =================== End" << std::endl;

				// Echo the data back to the client
				if (write(fds[i].fd, buffer, bytes_read) == -1) {
					std::cout << "write failed" << std::endl;
					exit(EXIT_FAILURE);
				}
			}
		}
	}
}
