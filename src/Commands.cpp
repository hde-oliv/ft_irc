#include <sstream>

#include "Channel.hpp"
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
std::string Server::join(pollfd p, Command &t) {
	Client			 *c = &clients[p.fd];
	std::stringstream ss;

	// TODO: Check later for ERR_BADCHANMASK

	if (t.args.size() < 1) {
		return needmoreparams(p, "JOIN");
	} else if (!validChannelName(t.args[0])) {
		return nosuchchannel(p, t.args[0]);
	}

	// If does not exist, it will be created
	Channel *ch = &channels[toIrcUpperCase(t.args[0])];

	// Channel setup
	if (ch->getName() == "") {
		ch->setName(t.args[0]);
		// Check if JOIN has password for the channel
	}

	// else if (ch->isBanned(p)) {
	// 	return bannedfromchan(p);
	// } else if (!ch->wasInvited(p)) {
	// 	return inviteonlychan(p);
	// } else if (t.args.size() > 1 && ch->password != t.args[1]) {
	// 	return badchannelkey(p);
	// } else if (ch->isFull()) {
	// 	return channelisfull(p);
	// } else if (c->getChannels().size() >= MAX_CHANNELS) {
	// 	return toomanychannels(p);
	// }

	ss << ":" << c->getNickname();
	ss << "!" << c->getUsername();
	ss << "@" << c->getHostname();
	ss << " JOIN :";
	ss << t.args[0];
	ss << "\r\n";

	ch->addClient(c);
	ch->broadcastToClients(ss.str());

	// TODO: Loop for join multiple channels in one command

	if (ch->getTopic() != "") {
		c->setSendData(topic(p, ch));
	} else {
		c->setSendData(notopic(p, ch));
	}

	return namreply(p, ch);
}
std::string Server::quit(pollfd p, Command &t) {
	Client			 *c = &clients[p.fd];
	std::stringstream ss;

	// :John!john123@irc.example.com QUIT :Client exited unexpectedly

	ss << ":" << c->getNickname();
	if (t.args.size()) {
		ss << " QUIT " << t.args[0];
	} else {
		ss << " QUIT :Gone to have lunch";
	}
	ss << "\r\n";

	broadcastMessage(p, ss.str());
	c->setToDisconnect(true);

	return "";
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

	std::string uppercaseNickname = toIrcUpperCase(nickname);

	for (; it != clients.end(); it++) {
		if (toIrcUpperCase((it->second).getNickname()) == uppercaseNickname)
			return true;
	}
	return false;
}

bool Server::validChannelName(std::string name) {
	if (name.length() < 1) return false;

	std::string prefixes = "&#!+";

	if (prefixes.find(name.at(0)) == std::string::npos) return false;
	if (name.length() > 50) return false;

	std::string insensitiveName = toIrcUpperCase(name);
	std::string forbiddenChars	= " \a,:";

	for (std::size_t i = 0; i < forbiddenChars.size(); i++) {
		if (insensitiveName.find(forbiddenChars.at(i)) != std::string::npos)
			return false;
	}
	return true;
};
