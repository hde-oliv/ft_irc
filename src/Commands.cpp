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
	std::stringstream ss;
	(void)t;
	Client	   *c	 = &clients[p.fd];
	Channel	*chan = NULL;
	std::string realName;
	ss << ":localhost ";
	try {
		realName = toValidChannelName(t.args[0]);
	} catch (std::exception &e) {
		ss << "403 " << c->getNickname() << " " << realName
		   << " :No such channel\r\n";
		return ss.str();
	}

	std::vector<Channel>::iterator it = channels.begin();
	while (it < channels.end()) {
		if ((*it).getName() == t.args[0]) {
			std::cout << "Join channel " << t.args[0] << std::endl;
			chan = &(*it);
		}
		it++;
	}
	if (chan == NULL) {
		channels.push_back(Channel(t.args[0], "", c->getHostname()));
	}
	chan = &channels.back();

	if (chan->getTopic().length() > 0)
		ss << "332 " << c->getNickname() << " " << realName << " :"
		   << chan->getTopic() << "\r\n";
	else
		ss << "332 " << c->getNickname() << " " << realName
		   << " :Simple topic\r\n";
	// ss << "331 " << c->getNickname() << " " << realName
	//    << " :no topic is set\r\n";
	return ss.str();
	/*
PASS :1234
USER hcduller * localhost :purple
NICK henrique
JOIN +teste 1234
	403	ERR_NOSUCHCHANNEL	"<channel name> :No such channel"
	461	ERR_NEEDMOREPARAMS	"<command> :Not enough parameters"
	474	ERR_BANNEDFROMCHAN	"<channel> :Cannot join channel (+b)"
	473	ERR_INVITEONLYCHAN	"<channel> :Cannot join channel (+i)"
	475	ERR_BADCHANNELKEY	"<channel> :Cannot join channel (+k)"
	471	ERR_CHANNELISFULL	"<channel> :Cannot join channel (+l)"
	ERR_BADCHANMASK //no definition found in rfc
	403	ERR_NOSUCHCHANNEL	"<channel name> :No such channel"
	405	ERR_TOOMANYCHANNELS	"<channel name> :You have joined too many \
								 channels"
	331	RPL_NOTOPIC			"<channel> :No topic is set"
	332	RPL_TOPIC			"<channel> :<topic>"
	*/
}
std::string Server::quit(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
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
