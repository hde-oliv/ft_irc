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

std::map<Client *, unsigned int> &Channel::getClients() { return clients; }

void Channel::setName(std::string name) { this->name = name; }
void Channel::setTopic(std::string topic) { this->topic = topic; };

void Channel::addClient(Client *c) { clients.insert(std::make_pair(c, 0)); }

void Channel::setPassword(std::string password) { this->password = password; }

void Channel::removeClient(Client *c) { clients.erase(c); }

// void Channel::removeOperator(Client *c) {
// 	(void)std::remove(operators.begin(), operators.end(), c);
// }

std::vector<Client *> &Channel::getOperators() { return operators; };

void Channel::broadcast(Client *sender, std::string message, bool toSend) {
	std::map<Client *, unsigned int>::iterator it = clients.begin();

	for (; it != clients.end(); it++) {
		if (it->first == sender && toSend) {
			(*it->first).setSendData(message);
		} else if (it->first != sender) {
			(*it->first).setSendData(message);
		}
	}
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
	this->clients.insert(std::make_pair(op, USER_OPERATOR));
	this->initialized = true;
}

void Channel::initialize(std::string name, Client *op) {
	this->name = name;
	this->clients.insert(std::make_pair(op, USER_OPERATOR));
	this->initialized = true;
}

void		 Channel::setUserLimit(unsigned int limit) { userLimit = limit; };
unsigned int Channel::getUserLimit() const { return userLimit; };

void Channel::setOperator(std::string clientNickname, bool newValue) {
	std::map<Client *, unsigned int>::iterator it =
		getClientByNick(clientNickname);
	if (it != clients.end()) {
		if (newValue)
			it->second |= USER_OPERATOR;
		else
			it->second &= (~USER_OPERATOR);
	}
};

void Channel::setMuted(std::string clientNickname, bool newValue) {
	std::map<Client *, unsigned int>::iterator it =
		getClientByNick(clientNickname);
	if (it != clients.end()) {
		if (newValue)
			it->second |= USER_MUTED;
		else
			it->second &= (~USER_MUTED);
	}
};
bool Channel::evalPassword(std::string psw) { return (password == psw); }

std::map<Client *, unsigned int>::iterator Channel::getClientByNick(
	std::string clientNickname) {
	std::map<Client *, unsigned int>::iterator it = clients.begin();
	while (it != clients.end()) {
		if (it->first->getNickname() == clientNickname) return (it);
		it++;
	}
	return (it);
};

void Channel::setBanMask(std::string newBanMask) { banMask = newBanMask; };
