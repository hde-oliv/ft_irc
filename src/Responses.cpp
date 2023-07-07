#include <sstream>

#include "Server.hpp"

std::string Server::needmoreparams(pollfd p, std::string command) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 461 " << c->getNickname();
	ss << " " << command << " :Not enough parameters";
	ss << "\r\n";

	return ss.str();
}

std::string Server::alreadyregistered(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 462 " << c->getNickname();
	ss << " :You may not reregister";
	ss << "\r\n";

	return ss.str();
}

std::string Server::passwdmismatch(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 464 " << c->getNickname();
	ss << " :Password incorrect";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nonicknamegiven(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 431 " << c->getNickname();
	ss << " :No nickname given";
	ss << "\r\n";

	return ss.str();
}

std::string Server::erroneusnickname(pollfd p, std::string nickname) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 432 " << c->getNickname();
	ss << " " << nickname << " :Erroneus nickname";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nicknameinuse(pollfd p, std::string nickname) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 433 " << c->getNickname();
	ss << " " << nickname << " :Nickname is already in use";
	ss << "\r\n";

	return ss.str();
}
std::string Server::welcome(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	// RPL_WELCOME 001
	ss << ":localhost 001 " << c->getNickname();
	ss << " :Welcome to the FT_IRC server " << c->getNickname();
	ss << "\r\n";

	// RPL_YOURHOST 002
	ss << ":localhost 002 " << c->getNickname();
	ss << " :Your host is localhost, running version 0.1";
	ss << "\r\n";

	// RPL_CREATED 003
	ss << ":localhost 003 " << c->getNickname();
	ss << " :This server was created " << creationDatetime;
	ss << "\r\n";

	// RPL_MYINFO 004
	ss << ":localhost 004 " << c->getNickname();
	ss << " localhost 0.1 iowstRb- biklmnopstvrRcCNuMTD";
	ss << "\r\n";

	return ss.str();
}

std::string Server::motd(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 375 " << c->getNickname();
	ss << " :- localhost Message of the day -";
	ss << "\r\n";

	ss << ":localhost 372 " << c->getNickname();
	ss << " :- Welcome to FT_IRC!";
	ss << "\r\n";

	ss << ":localhost 376 " << c->getNickname();
	ss << " :End of MOTD command.";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nooperhost(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 491 " << c->getNickname();
	ss << " :No O-lines for your host";
	ss << "\r\n";

	return ss.str();
}

std::string Server::youreoper(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 381 " << c->getNickname();
	ss << " :You are now an IRC operator";
	ss << "\r\n";

	return ss.str();
}

std::string Server::unknowncommand(pollfd p, std::string command) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 421 " << c->getNickname();
	ss << " " << command;
	ss << " :Unknown command";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nosuchchannel(pollfd p, std::string name) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 403 " << c->getNickname();
	ss << " " << name;
	ss << " :No such channel";
	ss << "\r\n";

	return ss.str();
}

std::string Server::topic(pollfd p, Channel *ch) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 332 " << c->getNickname();
	ss << " " << ch->getName();
	ss << " :" << ch->getTopic();
	ss << "\r\n";

	return ss.str();
}

std::string Server::notopic(pollfd p, Channel *ch) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 331 " << c->getNickname();
	ss << " " << ch->getName();
	ss << " :No topic is set";
	ss << "\r\n";

	return ss.str();
}

std::string Server::namreply(pollfd p, Channel *ch) {
	std::stringstream	  ss;
	Client			   *c	  = &clients[p.fd];
	std::vector<Client *> cli = ch->getClients();

	// TODO: Handle a lot of names

	ss << ":localhost 353 " << c->getNickname();
	ss << " = " << ch->getName();
	ss << " :" << cli[0]->getNickname();
	ss << "\r\n";

	// TODO: Consider the RFC for 366 implementation later

	ss << ":localhost 366 " << c->getNickname();
	ss << " " << ch->getName();
	ss << " :End of NAMES list";
	ss << "\r\n";

	return ss.str();
}
