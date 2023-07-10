#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>

#include "Client.hpp"

#define MAX_CHANNEL_NAME 200
#define CHANNEL_PREFIX "&#"
#define CHANNEL_INVALID_CHARS " ,"
#define USER_CHANNEL_LIMIT 10

#include <vector>

class Channel {
	public:
	Channel(void);
	~Channel(void);

	std::string getName() const;
	std::string getTopic() const;

	std::vector<Client *> getClients() const;
	std::vector<Client *> getOperators() const;

	bool validatePsw(std::string psw);

	bool setPassword(std::string newPsw);

	void setName(std::string name);
	void setTopic(std::string topic);

	void	setOwner(Client *c);
	Client *getOwner();

	void addClient(Client *c);
	void removeClient(Client *c);

	void addOperator(Client *c);
	void removeOperator(Client *c);

	void broadcastToClients(std::string message);

	bool isOperator(Client *c);

	private:
	Client			   *owner;
	std::string			  name;
	std::string			  topic;
	std::string			  host;
	std::string			  password;
	std::vector<Client *> operators;
	std::vector<Client *> clients;
	unsigned int		  modes;
	int					  limit;
};

#endif
