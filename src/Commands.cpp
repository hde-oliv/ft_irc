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

	c->setKnowPassword(true);
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

std::string Server::nick(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	Client			*c = &clients[p.fd];

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

std::string Server::oper(pollfd p, Tokens &tks) {
	Tokens::iterator it;
	Client			*c = &clients[p.fd];

	if (tks.size() < 3) {
		return needmoreparams(p, tks[1]);
	} else if (tks[2] != OPER_PASS) {
		return passwdmismatch(p);
	} else if (tks[1] != OPER_USER) {
		return nooperhost(p);
	}

	c->setOp(true);

	// TODO: Send MODE +o
	return youreoper(p);
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

// Utils
bool Server::validNickname(std::string nickname) {
	if (nickname.empty() || isdigit(nickname[0])) {
		return false;
	}

	std::string disallowedChars = " ,*?!@$.#&:\r\n\0\a";
	for (size_t i = 0; i < nickname.length(); ++i) {
		if (disallowedChars.find(nickname[i]) != std::string::npos) {
			return false;
		}
	}

	return true;
}

bool Server::nicknameAlreadyExists(std::string nickname) {
	std::map<int, Client>::iterator it = clients.begin();

	std::string uppercaseNickname = toUppercase(nickname);

	for (; it != clients.end(); it++) {
		if (toUppercase((it->second).getNickname()) == uppercaseNickname)
			return true;
	}
	return false;
}
