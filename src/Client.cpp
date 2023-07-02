#include "Client.hpp"

Client::Client() {}

Client::~Client() {
	fd			 = 0;
	registration = 0;
	knowPassword = false;
	welcome		 = false;
}

int			Client::getFd() const { return fd; }
int			Client::getRegistration() const { return registration; }
bool		Client::getWelcome() const { return welcome; }
bool		Client::getKnowPassword() const { return knowPassword; }
std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
std::string Client::getHostname() const { return hostname; }
std::string Client::getReadData() const { return readData; }
std::string Client::getSendData() const { return sendData; }
std::string Client::getServername() const { return servername; }
std::string Client::getRealname() const { return realname; }

void Client::setWelcome() { welcome = true; }
void Client::setKnowPassword() { knowPassword = true; }
void Client::setRegistration(int flag) { registration |= flag; }
void Client::setFd(int fd) { this->fd = fd; }
void Client::setUsername(std::string name) { username = name; }
void Client::setHostname(std::string name) { hostname = name; }
void Client::setRealname(std::string name) { realname = name; }
void Client::setServername(std::string name) { servername = name; }
void Client::setSendData(std::string data) { sendData = data; }
void Client::setReadData(std::string data) { readData += data; }

void Client::resetAllData() {
	sendData.clear();
	readData.clear();
}

void Client::resetSendData() { sendData.clear(); }
void Client::resetReadData() { readData.clear(); }
