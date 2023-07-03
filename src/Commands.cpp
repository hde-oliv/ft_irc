#include "Server.hpp"

std::string Server::pass(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	Client			*c = &clients[p.fd];

	if (tks.size() == 1) {
		return needmoreparams(p, "PASS");  // ERR_NEEDMOREPARAMS 461
	} else if (c->getRegistration() & PASS_FLAG) {
		return alreadyregistered(p);  // ERR_ALREADYREGISTRED 462
	} else if (tks[1].substr(1) != password) {
		return passwdmismatch(p);  // ERR_PASSWDMISMATCH 464
	}

	(void)tks;

	c->setKnowPassword();
	c->setRegistration(PASS_FLAG);
	c->resetReadData();

	return "";
}

std::string Server::user(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	Client			*c = &clients[p.fd];

	if (tks.size() < 5) {
		return needmoreparams(p, "USER");  // ERR_NEEDMOREPARAMS 461
	} else if (c->getRegistration() & USER_FLAG) {
		return alreadyregistered(p);  // ERR_ALREADYREGISTRED 462
	}

	c->setUsername(tks[1]);
	c->setHostname(tks[2]);
	c->setServername(tks[3]);
	c->setRealname(tks[4]);
	c->setRegistration(USER_FLAG);
	c->resetReadData();

	return "";
}

// TODO: Write these functions later
bool validNickname(std::string s) {
	(void)s;
	return true;
}

bool nicknameAlreadyExists(std::string s) {
	(void)s;
	return false;
}

std::string Server::nick(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	Client			*c = &clients[p.fd];

	// ERR_NONICKNAMEGIVEN             ERR_ERRONEUSNICKNAME
	// ERR_NICKNAMEINUSE               ERR_NICKCOLLISION

	// NOTE: Verify if the NICKCOLLISION is necessary since
	// the ft_irc does not handle server-server communication

	// TODO: Refactor to handle Prefix NICK command

	if (tks.size() == 1) {
		return nonicknamegiven(p);	// ERR_NONICKNAMEGIVEN 431
	} else if (!validNickname(tks[1])) {
		return erroneusnickname(p, tks[1]);	 // ERR_ERRONEUSNICKNAME 432
	} else if (nicknameAlreadyExists(tks[1])) {
		return nicknameinuse(p, tks[1]);  // ERR_NICKNAMEINUSE 433
	}
	c->setNickname(tks[1]);
	c->setRegistration(NICK_FLAG);
	c->resetReadData();

	return "";
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
