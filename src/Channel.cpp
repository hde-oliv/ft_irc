#include "Channel.hpp"

#include <stdexcept>

#include "Utils.hpp"

Channel::Channel() {
	host		= "localhost";
	creator		= NULL;
	userLimit	= USER_CHANNEL_LIMIT;
	initialized = false;
}

Channel::~Channel() {}

std::string Channel::getName() const { return name; };
std::string Channel::getTopic() const { return topic; }

void Channel::setInitialized(bool value) { initialized = value; }
bool Channel::isInitialized() { return initialized; }

std::vector<Client *> Channel::getClients() const { return clients; }
std::vector<Client *> Channel::getOperators() const { return operators; }

void Channel::setName(std::string name) { this->name = name; }
void Channel::setTopic(std::string topic) { this->topic = topic; };

void Channel::addClient(Client *c) { clients.push_back(c); }
void Channel::addOperator(Client *c) { operators.push_back(c); }

std::string Channel::getPassword() const { return password; }
void Channel::setPassword(std::string password) { this->password = password; }

bool Channel::isOperator(Client *c) {
	return std::find(operators.begin(), operators.end(), c) != operators.end();
}

void Channel::removeClient(Client *c) {
	(void)std::remove(clients.begin(), clients.end(), c);
}

void Channel::removeOperator(Client *c) {
	(void)std::remove(operators.begin(), operators.end(), c);
}

void Channel::broadcastToClients(std::string message) {
	std::vector<Client *>::iterator it = clients.begin();
	for (; it != clients.end(); it++) {
		(*it)->setSendData(message);
	}
}

bool Channel::validatePassword(std::string password) {
	if (password.find(',') != std::string::npos) return false;
	return true;
}

void Channel::setCreator(Client *c) { creator = c; };

Client *Channel::getCreator() { return creator; };

void Channel::toggleMode(char mode, bool on) {
	if (on)
		modes.insert(mode);
	else
		modes.erase(mode);
}

void Channel::initialize(std::string name, std::string password, Client *op) {
	this->name	   = name;
	this->password = password;
	this->addOperator(op);
	this->initialized = true;
}

void Channel::initialize(std::string name, Client *op) {
	this->name = name;
	this->addOperator(op);
	this->initialized = true;
}

void		 Channel::setUserLimit(unsigned int limit) { userLimit = limit; };
unsigned int Channel::getUserLimit() const { return userLimit; };

void Channel::promoteOperator(std::string clientNickname) {
	std::cout << "(TODO)promote " << clientNickname << "to operator of " << name
			  << std::endl;
}

void Channel::demoteOperator(std::string clientNickname) {
	std::cout << "(TODO)demote " << clientNickname << "to regular user of "
			  << name << std::endl;
}
