#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <exception>
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
#define FORBIDDEN_USER_CHARS " ,*?!@$.#&:\r\n\0\a+"

class Server {
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
	void	broadcastMessage(pollfd sender, std::string message);
	pollfd &getAvailablePollFd();
	void	disconnectHandling();
	void	unexpectedDisconnectHandling(pollfd p);

	std::map<std::string, Channel>::iterator getChannelByName(
		std::string channelName);

	std::string					   creationDatetime;
	std::string					   password;
	int							   port;
	int							   server_fd;
	std::map<int, Client>		   clients;
	std::map<std::string, Channel> channels;
	std::vector<pollfd>			   pollFds;
	struct sockaddr_in			   address;

	void executeClientMessage(pollfd p, std::string msg);
	void pass(pollfd p, Command &t);
	void user(pollfd p, Command &t);
	void nick(pollfd p, Command &t);
	void quit(pollfd p, Command &t);
	void oper(pollfd p, Command &t);
	void ping(pollfd p, Command &t);
	void join(pollfd p, Command &t);
	void who(pollfd p, Command &t);
	void whowas(pollfd p, Command &t);
	void whois(pollfd p, Command &t);
	void mode(pollfd p, Command &t);
	void privmsg(pollfd p, Command &t);

	void channelMode(pollfd p, Command &t);

	void recvLoop(pollfd p);

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
	std::string unknowncommand(pollfd p, std::string command);
	std::string nosuchchannel(pollfd p, std::string name);
	std::string topic(pollfd p, Channel *ch);
	std::string notopic(pollfd p, Channel *ch);
	std::string namreply(pollfd p, Channel *ch);
	std::string whoreply(pollfd p, Channel *ch);
	std::string nosuchserver(pollfd p, std::string name);
	std::string unknownmode(pollfd p, char c);

	bool validNickname(std::string nickname);
	bool nicknameAlreadyExists(std::string nickname);
	bool validChannelName(std::string name);
};

#endif
