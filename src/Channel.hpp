#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <map>
#include <string>

#include "Client.hpp"

#define MAX_CHANNEL_NAME 200
#define CHANNEL_PREFIX "&#!+"
#define CHANNEL_INVALID_CHARS " ,"
#define USER_CHANNEL_LIMIT 10
#define CHANNEL_MODES "opsitnmlbvk"
#define USER_MODES "iswov"

#define USER_INVISIBLE 0x1
#define USER_NOTICES 0x2
#define USER_WALLOP 0x4
#define USER_OPERATOR 0x8
#define USER_MUTED 0x16
/*
	Channel modes
	o - give/take channel operator privileges;
	p - private channel flag;
	s - secret channel flag;
	i - invite-only channel flag;
	t - topic settable by channel operator only flag;
	n - no messages to channel from clients on the outside;
	m - moderated channel;
	l - set the user limit to channel;
	b - set a ban mask to keep users out;
	v - give/take the ability to speak on a moderated channel;
	k - set a channel key (password).

	User modes:
	i - marks a users as invisible;
	s - marks a user for receipt of server notices;
	w - user receives wallops;
	o - operator flag.
*/
#include <set>
#include <vector>

class Client;

class Channel {
	public:
	Channel(void);
	~Channel(void);

	std::string			   getName() const;
	std::string			   getTopic() const;
	Client				  *getCreator();
	unsigned int		   getUserLimit() const;
	std::vector<Client *> &getOperators();

	std::map<Client *, unsigned int> &getClients();

	std::map<Client *, unsigned int>::iterator getClientByNick(
		std::string clientNickname);

	std::pair<Client *, unsigned int &> getClientByNick2(
		std::string clientNickname);

	bool isInitialized();
	bool evalPassword(std::string psw);
	void setPassword(std::string password);
	void setName(std::string name);
	void setTopic(std::string topic);
	void setCreator(Client *c);
	void setUserLimit(unsigned int limit);
	void setInitialized(bool value);

	void addClient(Client *c);
	void removeClient(Client *c);
	void setOperator(std::string clientNickname, bool newValue);
	void setMuted(std::string clientNickname, bool newValue);
	void broadcast(Client *sender, std::string message, bool toSend);
	void toggleMode(char mode, bool on);
	void setBanMask(std::string newBanMask);
	void initialize(std::string name, std::string password, Client *op);
	void initialize(std::string name, Client *op);

	private:
	Client							*creator;
	std::string						 name;
	std::string						 topic;
	std::string						 host;
	std::string						 password;
	std::vector<Client *>			 operators;
	std::set<char>					 modes;
	std::string						 banMask;
	unsigned int					 userLimit;
	bool							 initialized;
	std::map<Client *, unsigned int> clients;
};

#endif
