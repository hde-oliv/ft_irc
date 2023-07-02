#include "Client.hpp"
#include "Server.hpp"

std::string Server::welcome(pollfd p) {
	(void)p;
	return "";
}

std::string Server::pass(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	Client			*c = &clients[p.fd];

	if (tks.size() == 1) {
		// return ERR_NEEDMOREPARAMS 461
	} else if (c->getServerPassword() != "") {
		// return ERR_ALREADYREGISTRED 462
	} else if (tks[1].substr(1) != password) {
		// return ERR_PASSWDMISMATCH 464
	}

	c->setServerPassword(tks[1].substr(1));
	c->setRegistration(PASS_FLAG);
	c->resetReadData();
	return "";
}

std::string Server::user(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	(void)p;

	std::cout << "USER COMMAND" << std::endl;
	for (it = tks.begin(); it != tks.end(); it++) {
		std::cout << *it << std::endl;
	}

	return "U";
}

std::string Server::nick(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	(void)p;

	std::cout << "NICK COMMAND" << std::endl;
	for (it = tks.begin(); it != tks.end(); it++) {
		std::cout << *it << std::endl;
	}

	return "N";
}

std::string Server::quit(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	(void)p;

	std::cout << "QUIT COMMAND" << std::endl;
	for (it = tks.begin(); it != tks.end(); it++) {
		std::cout << *it << std::endl;
	}

	return "Q";
}
