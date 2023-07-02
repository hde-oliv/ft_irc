#include "Client.hpp"

Client::Client() {}

Client::~Client() {
	fd			 = 0;
	registration = 0;
	welcome		 = false;
}

int			Client::getFd() const { return fd; }
int			Client::getRegistration() const { return registration; }
bool		Client::getWelcome() const { return welcome; }
std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
std::string Client::getHost() const { return host; }
std::string Client::getReadData() const { return readData; }
std::string Client::getSendData() const { return sendData; }
std::string Client::getServerPassword() const { return serverPassword; }

void Client::setHost(std::string host) { this->host = host; }
void Client::setFd(int fd) { this->fd = fd; }
void Client::setSendData(std::string data) { sendData = data; }
void Client::setReadData(std::string data) { readData += data; }
void Client::setWelcome() { welcome = true; }
void Client::setRegistration(int flag) { registration |= flag; }
void Client::setServerPassword(std::string password) {
	serverPassword = password;
}

void Client::resetAllData() {
	sendData.clear();
	readData.clear();
}

void Client::resetSendData() { sendData.clear(); }
void Client::resetReadData() { readData.clear(); }
