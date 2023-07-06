#include "Channel.hpp"

#include <stdexcept>

#include "Utils.hpp"

Channel::Channel() {}

Channel::~Channel() {}

Channel::Channel(std::string name, std::string topic, std::string host) {
	setName(name);
	setTopic(topic);
	this->host = host;	// needs validation
};

std::string Channel::getName() { return this->name; };

void Channel::setName(std::string newName) {
	this->name = toValidChannelName(newName);
};

std::string Channel::getTopic() { return topic; };
void		Channel::setTopic(std::string newTopic) { this->topic = newTopic; };
