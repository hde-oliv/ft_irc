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
	typedef std::vector<std::string> Tokens;

	public:
	Server(void);
	Server(std::string const &password, int const &port);
	~Server(void);
	void startServer();

	private:
	void	setupSocket();
	void	clientEventHandling();
	void	serverEventHandling();
	void	newClientHandling();
	void	readFromClient(pollfd p);
	void	sendToClient(pollfd p);
	void	ejectClient(int clientFd, int reason);
	void	ejectAllClients();
	pollfd &getAvailablePollFd();

	std::string			  creationDatetime;
	std::string			  password;
	int					  port;
	int					  server_fd;
	int					  poll_index;
	std::map<int, Client> clients;
	std::vector<Channel>  channels;
	struct pollfd		  pollfds[MAX_CLIENTS];
	struct sockaddr_in	  address;

	std::string executeClientMessage(pollfd p, std::string msg);
	void		setupCommandMap();
	std::string pass(pollfd p, Tokens &tks);
	std::string user(pollfd p, Tokens &tks);
	std::string nick(pollfd p, Tokens &tks);
	std::string quit(pollfd p, Tokens &tks);
	std::string motd(pollfd p);
	std::string welcome(pollfd p);
};

#endif
