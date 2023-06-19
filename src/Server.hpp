#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server {
	public:
	Server( void );
	Server( std::string const &password, unsigned int const &port );
	Server( Server const &ref );
	~Server( void );
	Server &operator=( Server const &ref );

	private:
	std::string	 password;
	unsigned int port;
};

#endif
