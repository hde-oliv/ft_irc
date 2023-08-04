#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Client.hpp"

#define MAX_CHANNEL_NAME 200
#define CHANNEL_PREFIX "&#!+"
#define CHANNEL_INVALID_CHARS " ,"
#define USER_CHANNEL_LIMIT 10

#define USER_INVISIBLE 0b1
#define USER_NOTICES 0b10
#define USER_WALLOP 0b100
#define USER_OPERATOR 0b1000
#define USER_SPEAKER 0b10000

class Client;

class Channel {
	public:
	Channel(void);
	~Channel(void);

	std::string		getName() const;
	std::string		getTopic() const;
	Client		   *getCreator();
	unsigned int	getUserLimit() const;
	std::set<char> &getMode();

	std::map<Client *, unsigned int> &getClients();

	std::map<Client *, unsigned int>::iterator getClientByNick(
		std::string clientNickname);

	std::string getStrModes();

	bool isInitialized();
	bool evalPassword(std::string psw);
	void setPassword(std::string password);
	void setName(std::string name);
	void setTopic(std::string topic);
	void setCreator(Client *c);
	void setUserLimit(unsigned int limit);
	void setInitialized(bool value);
	bool isInvited(std::string nickname);
	void addInvite(std::string nickname);
	void removeInvited(std::string nickname);
	void asureOperator();

	void addClient(Client *c);
	void removeClient(Client *c);
	void removePassword();
	bool setOperator(std::string clientNickname, bool newValue);
	void setSpeaker(std::string clientNickname, bool newValue);
	void broadcast(Client *sender, std::string message, bool toSend);
	bool toggleMode(char mode, bool on);
	void initialize(std::string name, std::string password, Client *op);
	void initialize(std::string name, Client *op);

	private:
	Client						  *creator;
	std::string						 name;
	std::string						 topic;
	std::string						 host;
	std::string						 password;
	std::set<char>					 modes;
	unsigned int					 userLimit;
	bool							 initialized;
	std::map<Client *, unsigned int> clients;
	std::set<std::string>			 invited;
};

#endif
