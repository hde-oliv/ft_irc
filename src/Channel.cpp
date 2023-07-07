#include "Channel.hpp"

#include <stdexcept>

#include "Utils.hpp"

Channel::Channel() { host = "localhost"; }

Channel::~Channel() {}

std::string Channel::getName() const { return name; };
std::string Channel::getTopic() const { return topic; }

std::vector<Client *> Channel::getClients() const { return clients; }

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
