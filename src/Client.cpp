#include "Client.hpp"

Client::Client() {}

Client::~Client() {}

std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
std::string Client::getHost() const { return host; }
std::string Client::getReadData() const { return readData; }
std::string Client::getSendData() const { return sendData; }

void Client::setHost(std::string host) { this->host = host; }
void Client::setFd(int fd) { this->fd = fd; }
void Client::setSendData(std::string data) { sendData = data; }
void Client::setReadData(std::string data) { readData += data; }

void Client::resetData() {
	sendData.clear();
	readData.clear();
}
