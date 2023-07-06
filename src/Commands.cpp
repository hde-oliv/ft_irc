#include "Server.hpp"

std::string Server::pass(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() == 0) {
		return needmoreparams(p, "PASS");  // ERR_NEEDMOREPARAMS 461
	} else if (c->getRegistration() & PASS_FLAG) {
		return alreadyregistered(p);  // ERR_ALREADYREGISTRED 462
	} else if (t.args[0].substr(1) != password) {
		return passwdmismatch(p);  // ERR_PASSWDMISMATCH 464
	}

	c->setKnowPassword(true);
	c->setRegistration(PASS_FLAG);
	c->resetReadData();

	return "";
}

std::string Server::user(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() < 4) {
		return needmoreparams(p, "USER");  // ERR_NEEDMOREPARAMS 461
	} else if (c->getRegistration() & USER_FLAG) {
		return alreadyregistered(p);  // ERR_ALREADYREGISTRED 462
	}

	c->setUsername(t.args[0]);
	c->setHostname(t.args[1]);
	c->setServername(t.args[2]);
	c->setRealname(t.args[3]);
	c->setRegistration(USER_FLAG);
	c->resetReadData();

	return "";
}

std::string Server::nick(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() == 0) {
		return nonicknamegiven(p);	// ERR_NONICKNAMEGIVEN 431
	} else if (!validNickname(t.args[0])) {
		return erroneusnickname(p, t.args[0]);	// ERR_ERRONEUSNICKNAME 432
	} else if (nicknameAlreadyExists(t.args[0])) {
		return nicknameinuse(p, t.args[0]);	 // ERR_NICKNAMEINUSE 433
	}
	c->setNickname(t.args[0]);
	c->setRegistration(NICK_FLAG);
	c->resetReadData();

	return "";
}

std::string Server::oper(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() < 2) {
		return needmoreparams(p, t.cmd);
	} else if (t.args[1] != OPER_PASS) {
		return passwdmismatch(p);
	} else if (t.args[0] != OPER_USER) {
		return nooperhost(p);
	}

	c->setOp(true);
	c->resetReadData();

	// TODO: Send MODE +o
	return youreoper(p);
}

std::string Server::quit(pollfd p, Command &t) {
	Client			 *c = &clients[p.fd];
	std::stringstream ss;

	// :John!john123@irc.example.com QUIT :Client exited unexpectedly

	ss << ":" << c->getNickname();
	if (t.args.size()) {
		ss << " QUIT " << t.args[1];
	} else {
		ss << " QUIT :Gone to have lunch";
	}
	ss << "\r\n";

	broadcastMessage(ss.str());
	c->setToDisconnect(true);

	return ""
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
