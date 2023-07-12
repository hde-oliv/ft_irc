#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>

#include "Client.hpp"

#define MAX_CHANNEL_NAME 200
#define CHANNEL_PREFIX "&#!+"
#define CHANNEL_INVALID_CHARS " ,"
#define USER_CHANNEL_LIMIT 10
#define CHANNEL_MODES "opsitnmlbvk"
#define USER_MODES "iswo"
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

class Channel {
	public:
	Channel(void);
	~Channel(void);

	std::string			  getName() const;
	std::string			  getTopic() const;
	std::vector<Client *> getClients() const;
	std::vector<Client *> getOperators() const;
	Client				 *getCreator();
	unsigned int		  getUserLimit() const;
	std::string			  getPassword() const;

	bool isOperator(Client *c);
	bool isInitialized();
	bool validatePassword(std::string password);
	void setPassword(std::string password);
	void setName(std::string name);
	void setTopic(std::string topic);
	void setCreator(Client *c);
	void setUserLimit(unsigned int limit);
	void setInitialized(bool value);

	void addClient(Client *c);
	void removeClient(Client *c);
	void addOperator(Client *c);
	void removeOperator(Client *c);
	void promoteOperator(std::string clientNickname);
	void demoteOperator(std::string clientNickname);
	void broadcastToClients(std::string message);
	void toggleMode(char mode, bool on);
	void initialize(std::string name, std::string password, Client *op);
	void initialize(std::string name, Client *op);

	private:
	Client				 *creator;
	std::string			  name;
	std::string			  topic;
	std::string			  host;
	std::string			  password;
	std::vector<Client *> operators;
	std::vector<Client *> clients;
	std::set<char>		  modes;
	std::string			  banMask;
	unsigned int		  userLimit;
	bool				  initialized;
};

#endif
