#include "Channel.hpp"

#include <stdexcept>

#include "Utils.hpp"

Channel::Channel() {
	host  = "localhost";
	owner = NULL;
}

Channel::~Channel() {}

std::string Channel::getName() const { return name; };
std::string Channel::getTopic() const { return topic; }

std::vector<Client *> Channel::getClients() const { return clients; }
std::vector<Client *> Channel::getOperators() const { return operators; }

void Channel::setName(std::string name) { this->name = name; }
void Channel::setTopic(std::string topic) { this->topic = topic; };

void Channel::addClient(Client *c) { clients.push_back(c); }
void Channel::addOperator(Client *c) { operators.push_back(c); }

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

bool Channel::validatePsw(std::string psw) { return (psw == password); }

// Try to set new password, returns a boolean value indicating its success or
// failure
bool Channel::setPassword(std::string newPsw) {
	// there might be another constrains, but I found none in the RFC.
	// If ',' is not removed from the password, where is no way to parse
	// parameters.
	if (newPsw.find(',') != std::string::npos) return false;
	password = newPsw;
	return true;
};
void Channel::setOwner(Client *c) { owner = c; };

Client *Channel::getOwner() { return owner; };