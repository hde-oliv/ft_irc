#include <sstream>

#include "Client.hpp"
#include "Server.hpp"

std::string Server::welcome(pollfd p) {
	std::stringstream ss;
	Client			 *c = &clients[p.fd];

	// RPL_WELCOME 001
	ss << ":localhost 001" << c->getNickname();
	ss << ":Welcome to the FT_IRC server " << c->getNickname();
	ss << "\r\n";

	// RPL_YOURHOST 002
	ss << ":localhost 002" << c->getNickname();
	ss << ":Your host is localhost, running version 0.1";
	ss << "\r\n";

	// RPL_CREATED 003
	ss << ":localhost 003" << c->getNickname();
	ss << ":This server was created " << creationDatetime;
	ss << "\r\n";

	// RPL_MYINFO 004
	ss << ":localhost 004" << c->getNickname();
	ss << "localhost 0.1 iowstRb- biklmnopstvrRcCNuMTD";
	ss << "\r\n";

	return ss.str();
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
	Client			*c = &clients[p.fd];

	// ERR_NONICKNAMEGIVEN             ERR_ERRONEUSNICKNAME
	// ERR_NICKNAMEINUSE               ERR_NICKCOLLISION

	// NOTE: Verify if the NICKCOLLISION is necessary since
	// the ft_irc does not handle server-server communication

	// TODO: Refactor to handle Prefix NICK command

	if (tks.size() == 1) {
		// return ERR_NONICKNAMEGIVEN 431
	}
	// TODO: Implement the following functions
	// else if (!validNickname(tks[1])) {
	// return ERR_ERRONEUSNICKNAME 432
	// } else if (nicknameAlreadyExists(tks[1])) {
	// return ERR_NICKNAMEINUSE 433
	// }

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
