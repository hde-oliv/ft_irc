#include "Client.hpp"

Client::Client() {}

Client::~Client() {}

void Client::setHost(std::string host) { this->host = host; }
void Client::setFd(int fd) { this->fd = fd; }
