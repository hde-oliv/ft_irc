#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

class Server {
	public:
	Server( void );
	Server( std::string const &password, unsigned int const &port );
	Server( Server const &ref );
	~Server( void );
	Server &operator=( Server const &ref );
	void	startListening();

	private:
	std::string	 password;
	unsigned int port;
};

#endif
