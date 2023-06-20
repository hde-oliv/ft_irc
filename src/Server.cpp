#include "Server.hpp"

#include <asm-generic/socket.h>
#include <sys/socket.h>

#include <cstdlib>

Server::Server() {}

Server::Server( std::string const &password, unsigned int const &port ) {
	this->password = password;
	this->port	   = port;
}

Server::Server( Server const &ref ) {
	this->password = ref.password;
	this->port	   = ref.port;
}

Server::~Server() {}

Server &Server::operator=( const Server &ref ) {
	this->password = ref.password;
	this->port	   = ref.port;

	return *this;
}

void Server::startListening() {
	int				   serverfd, new_socket, valread;
	struct sockaddr_in address;
	int				   opt			= 1;
	int				   addrlen		= sizeof( address );
	char			   buffer[1024] = { 0 };
	std::string		   hello		= "Hello from server";

	serverfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( serverfd < 0 ) {
		std::cout << "socket failed" << std::endl;
		exit( EXIT_FAILURE );
	}

	if ( setsockopt( serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
					 sizeof( opt ) ) ) {
		std::cout << "setsockopt failed" << std::endl;
		exit( EXIT_FAILURE );
	}

	address.sin_family		= AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port		= htons( this->port );

	if ( bind( serverfd, (struct sockaddr *)&address, sizeof( address ) ) <
		 0 ) {
		std::cout << "setsockopt failed" << std::endl;
		exit( EXIT_FAILURE );
	}
	if ( listen( serverfd, 3 ) < 0 ) {
		std::cout << "setsockopt failed" << std::endl;
		exit( EXIT_FAILURE );
	}
	new_socket =
		accept( serverfd, (struct sockaddr *)&address, (socklen_t *)&addrlen );

	if ( new_socket < 0 ) {
		std::cout << "accept failed" << std::endl;
		exit( EXIT_FAILURE );
	}
	(void)valread;

	valread = read( new_socket, buffer, 1024 );
	send( new_socket, hello.c_str(), hello.length(), 0 );

	close( new_socket );
	shutdown( serverfd, SHUT_RDWR );
}
