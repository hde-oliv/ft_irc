#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"

#define MAX_CLIENTS 3
// Reasons for ajecting a client
#define LOSTCONNECTION 0
#define QUITED 1
#define KICKED 2

class Server {
	public:
	Server(void);
	Server(std::string const &password, int const &port);
	~Server(void);
	void startServer();

	private:
	void	setupSocket();
	void	clientEventHandling();
	void	newClientHandling();
	void	ejectClient(int clientFd, int reason);
	pollfd &getAvailablePollFd();

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
