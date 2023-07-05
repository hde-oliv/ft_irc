#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"
#include "Utils.hpp"

#define LOSTCONNECTION 0
#define QUITED 1
#define KICKED 2
#define MAX_CLIENTS 5

class Server {
	typedef std::vector<std::string> Tokens;

	public:
	Server(void);
	Server(std::string const &password, int const &port);
	~Server(void);
	void startServer();

	private:
	void setupSocket();
	void clientEventHandling();
	void serverEventHandling();
	void newClientHandling();
	void readFromClient(pollfd p);
	void sendToClient(pollfd p);
	void ejectClient(int clientFd, int reason);
	void ejectAllClients();

	std::string			  creationDatetime;
	std::string			  password;
	int					  port;
	int					  server_fd;
	std::map<int, Client> clients;
	std::vector<Channel>  channels;
	std::vector<pollfd>	  pollFds;
	struct sockaddr_in	  address;

	std::string executeClientMessage(pollfd p, std::string msg);
	void		setupCommandMap();
	std::string pass(pollfd p, Command &t);
	std::string user(pollfd p, Command &t);
	std::string nick(pollfd p, Command &t);
	std::string quit(pollfd p, Command &t);
	std::string oper(pollfd p, Command &t);

	std::string motd(pollfd p);
	std::string welcome(pollfd p);
	std::string needmoreparams(pollfd p, std::string command);
	std::string alreadyregistered(pollfd p);
	std::string passwdmismatch(pollfd p);
	std::string nonicknamegiven(pollfd p);
	std::string erroneusnickname(pollfd p, std::string nickname);
	std::string nicknameinuse(pollfd p, std::string nickname);
	std::string youreoper(pollfd p);
	std::string nooperhost(pollfd p);

	bool validNickname(std::string nickname);
	bool nicknameAlreadyExists(std::string nickname);
};

#endif
