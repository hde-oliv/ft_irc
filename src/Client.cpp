#include "Client.hpp"

#include "Channel.hpp"

Client::Client() {
	this->fd		   = 0;
	this->registration = 0;
	this->knowPassword = false;
	this->welcome	   = false;
	this->toDisconnect = false;
	this->op		   = false;
}

Client::~Client() {
	std::size_t i = 0;
	while (i < channels.size()) {
		channels.at(i)->removeClient(this);
		i++;
	}
}

int			Client::getFd() const { return fd; }
int			Client::getRegistration() const { return registration; }
bool		Client::getWelcome() const { return welcome; }
bool		Client::getKnowPassword() const { return knowPassword; }
bool		Client::getToDisconnect() const { return toDisconnect; }
bool		Client::getOp() const { return op; }
std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
std::string Client::getHostname() const { return hostname; }
std::string Client::getReadData() const { return readData; }
std::string Client::getSendData() const { return sendData; }
std::string Client::getServername() const { return servername; }
std::string Client::getRealname() const { return realname; }
std::string Client::getClientPrefix() {
	std::stringstream ss;

	ss << ":" << getNickname();
	ss << "!" << getUsername();
	ss << "@" << getHostname();
	return (ss.str());
};

void Client::setOp(bool value) { op = value; }
void Client::setWelcome(bool value) { welcome = value; }
void Client::setKnowPassword(bool value) { knowPassword = value; }
void Client::setRegistration(int flag) { registration |= flag; }
void Client::setFd(int fd) { this->fd = fd; }
void Client::setUsername(std::string name) { username = name; }
void Client::setHostname(std::string name) { hostname = name; }
void Client::setRealname(std::string name) { realname = name; }
void Client::setServername(std::string name) { servername = name; }
void Client::setSendData(std::string data) { sendData += data; }
void Client::setReadData(std::string data) { readData += data; }
void Client::setToDisconnect(bool value) { toDisconnect = value; }

void Client::setNickname(std::string name) {
	if (name.length() > 9) {
		nickname = name.substr(0, 9);
	} else {
		nickname = name;
	}
}

void Client::resetAllData() {
	sendData.clear();
	readData.clear();
}

void Client::resetSendData(int len) { sendData = sendData.substr(len); }
void Client::resetReadData() { readData.clear(); }
/*
bool operator<(const Client& lhs, const Client& rhs) {
	return lhs.getFd() < rhs.getFd();
}
bool operator!=(const Client& lhs, const Client& rhs) {
	return lhs.getFd() != rhs.getFd();
}
bool operator==(const Client& lhs, const Client& rhs) {
	return lhs.getFd() == rhs.getFd();
}
*/