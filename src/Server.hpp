#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"

#define MAX_CLIENTS 5

#include <map>
#include <vector>

class Server {
	public:
	Server(void);
	Server(std::string const &password, int const &port);
	~Server(void);
	void startServer();

	private:
	void setupSocket();
	void clientEventHandling();
	void serverEventHandling();

	std::string			  password;
	int					  port;
	int					  server_fd;
	int					  poll_index;
	std::map<int, Client> clients;
	std::vector<Channel>  channels;
	struct pollfd		  pollfds[MAX_CLIENTS];
	struct sockaddr_in	  address;
};

#endif
