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
	} else if (c->getRegistration() & PASS_FLAG) {
		// return ERR_ALREADYREGISTRED 462
	} else if (tks[1].substr(1) != password) {
		// return ERR_PASSWDMISMATCH 464
	}

	c->setKnowPassword();
	c->setRegistration(PASS_FLAG);
	c->resetReadData();

	return "";
}

std::string Server::user(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	Client			*c = &clients[p.fd];

	if (tks.size() < 5) {
		// return ERR_NEEDMOREPARAMS 461
	} else if (c->getRegistration() & USER_FLAG) {
		// return ERR_ALREADYREGISTERED 462
	}

	c->setUsername(tks[1]);
	c->setHostname(tks[2]);
	c->setServername(tks[3]);
	c->setRealname(tks[4]);
	c->setRegistration(USER_FLAG);
	c->resetReadData();

	return "";
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
