#include "Client.hpp"

Client::Client() {}

Client::~Client() {}

std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
std::string Client::getHost() const { return host; }

void Client::setHost(std::string host) { this->host = host; }
void Client::setFd(int fd) { this->fd = fd; }
