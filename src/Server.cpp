#include "Server.hpp"

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
