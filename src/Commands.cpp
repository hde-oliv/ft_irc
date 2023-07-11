#include <sys/poll.h>

#include <sstream>
#include <utility>
#include <vector>

#include "Channel.hpp"
#include "Server.hpp"
#include "Utils.hpp"

void Server::pass(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() == 0) {
		c->setSendData(needmoreparams(p, "PASS"));	// ERR_NEEDMOREPARAMS 461
	} else if (c->getRegistration() & PASS_FLAG) {
		c->setSendData(alreadyregistered(p));  // ERR_ALREADYREGISTRED 462
	} else if (t.args[0].substr(1) != password) {
		c->setSendData(passwdmismatch(p));	// ERR_PASSWDMISMATCH 464
	}

	c->setKnowPassword(true);
	c->setRegistration(PASS_FLAG);
}

void Server::user(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() < 4) {
		c->setSendData(needmoreparams(p, "USER"));	// ERR_NEEDMOREPARAMS 461
	} else if (c->getRegistration() & USER_FLAG) {
		c->setSendData(alreadyregistered(p));  // ERR_ALREADYREGISTRED 462
	}

	c->setUsername(t.args[0]);
	c->setHostname(t.args[1]);
	c->setServername(t.args[2]);
	c->setRealname(t.args[3].substr(1));
	c->setRegistration(USER_FLAG);
}

void Server::nick(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() == 0) {
		c->setSendData(nonicknamegiven(p));	 // ERR_NONICKNAMEGIVEN 431
	} else if (!validNickname(t.args[0])) {
		c->setSendData(
			erroneusnickname(p, t.args[0]));  // ERR_ERRONEUSNICKNAME 432
	} else if (nicknameAlreadyExists(t.args[0])) {
		c->setSendData(nicknameinuse(p, t.args[0]));  // ERR_NICKNAMEINUSE 433
	}
	c->setNickname(t.args[0]);
	c->setRegistration(NICK_FLAG);
}

void Server::oper(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() < 2) {
		c->setSendData(needmoreparams(p, t.cmd));
	} else if (t.args[1] != OPER_PASS) {
		c->setSendData(passwdmismatch(p));
	} else if (t.args[0] != OPER_USER) {
		c->setSendData(nooperhost(p));
	}

	c->setOp(true);
	c->setSendData(youreoper(p));
	// TODO: Send MODE +o
}

void Server::join(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	std::vector<std::pair<std::string, std::string> > chanPsw;

	// TODO: Check later for ERR_BADCHANMASK

	if (t.args.size() < 1) return (c->setSendData(needmoreparams(p, "JOIN")));

	// if size == 1, only channels
	if (t.args.size() == 1) {
		std::vector<std::string> chanNames = splitWithToken(t.args[0], ',');
		std::vector<std::string>::iterator it_name = chanNames.begin();
		while (it_name != chanNames.end()) {
			std::stringstream reponseStream;
			if (!validChannelName(*it_name))
				return (c->setSendData(nosuchchannel(p, *it_name)));
			Channel *ch = &channels[toIrcUpperCase(*it_name)];
			if (ch->getName() == "") {
				ch->setName(*it_name);
				ch->setOwner(c);
			} else {
				ch->addClient(c);
			}
			// after implementing BANS, this line should check for bans!
			reponseStream << c->getClientPrefix();
			reponseStream << " JOIN :";
			reponseStream << *it_name;
			reponseStream << "\r\n";

			ch->broadcastToClients(reponseStream.str());
			if (ch->getTopic() != "") {
				c->setSendData(topic(p, ch));
			} else {
				c->setSendData(notopic(p, ch));
			}
			c->setSendData(namreply(p, ch));
			it_name++;
		}
		return;
	}
	// if size == 2, channels and passwords
	if (t.args.size() == 2) {
		std::vector<std::string> chanNames = splitWithToken(t.args[0], ',');
		std::vector<std::string> psws	   = splitWithToken(t.args[1], ',');
		if (chanNames.size() != psws.size())
			return (c->setSendData(needmoreparams(p, "JOIN")));

		std::vector<std::string>::iterator it_name = chanNames.begin();
		std::vector<std::string>::iterator it_psw  = psws.begin();
		while (it_name != chanNames.end()) {
			std::stringstream reponseStream;
			if (!validChannelName(*it_name))
				return (c->setSendData(nosuchchannel(p, *it_name)));
			Channel *ch = &channels[toIrcUpperCase(*it_name)];
			if (ch->getName() == "") {
				ch->setName(*it_name);
				if (!ch->setPassword(*it_psw)) {
					return (c->setSendData(needmoreparams(p, "JOIN")));
				}
				ch->setOwner(c);
			} else {
				ch->addClient(c);
			}
			if (!ch->validatePsw(*it_psw)) {
				return (c->setSendData(needmoreparams(p, "JOIN")));
			}
			// after implementing BANS, this line should check for bans!
			reponseStream << c->getClientPrefix();
			reponseStream << " JOIN :";
			reponseStream << *it_name;
			reponseStream << "\r\n";
			ch->broadcastToClients(reponseStream.str());
			if (ch->getTopic() != "") {
				c->setSendData(topic(p, ch));
			} else {
				c->setSendData(notopic(p, ch));
			}
			c->setSendData(namreply(p, ch));
			it_name++;
			it_psw++;
		}
		return;
	}
	return (c->setSendData(nosuchchannel(p, t.args[0])));
	/*
	ERR_BANNEDFROMCHAN
	ERR_BADCHANNELKEY
	ERR_BADCHANMASK
	ERR_TOOMANYCHANNELS
	ERR_NEEDMOREPARAMS
	ERR_INVITEONLYCHAN
	ERR_CHANNELISFULL
	ERR_NOSUCHCHANNEL
	RPL_TOPIC
	else if (ch->isBanned(p)) {
		return bannedfromchan(p);
	} else if (!ch->wasInvited(p)) {
		return inviteonlychan(p);
	} else if (t.args.size() > 1 && ch->password != t.args[1]) {
		return badchannelkey(p);
	} else if (ch->isFull()) {
		return channelisfull(p);
	} else if (c->getChannels().size() >= MAX_CHANNELS) {
		return toomanychannels(p);
	}
	*/
}

void Server::who(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	// NOTE: Not defined in RFC
	if (t.args.size() < 1) {
		c->setSendData(needmoreparams(p, "WHO"));
	}

	std::map<std::string, Channel>::iterator it;

	for (it = channels.begin(); it != channels.end(); it++) {
		if (it->first == toIrcUpperCase(t.args[0])) {
			c->setSendData(whoreply(p, &(it->second)));
			return;
		}
	}

	// TODO: Check later what other servers respond when the Channel parameter
	// does not exist
	c->setSendData(nosuchserver(p, t.args[0]));
}

void Server::whois(pollfd p, Command &t) {
	(void)p;
	(void)t;
}

void Server::whowas(pollfd p, Command &t) {
	(void)p;
	(void)t;
}

void Server::quit(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	// TODO: Check if this broadcast is only on the channel
	ss << ":" << c->getNickname();
	if (t.args.size()) {
		ss << " QUIT " << t.args[0];
	} else {
		ss << " QUIT :Gone to have lunch";
	}
	ss << "\r\n";

	broadcastMessage(p, ss.str());
	c->setToDisconnect(true);
}

void Server::ping(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	ss << ":localhost PONG localhost";
	if (t.args.size()) {
		ss << " :" << t.args[0];
	}
	ss << "\r\n";

	c->setSendData(ss.str());
}

void Server::mode(pollfd p, Command &t) {
	Client	   *c		  = &clients[p.fd];
	std::string ch_prefix = CHANNEL_PREFIX;
	// identify if command applies to channel or client
	if (ch_prefix.find(t.args[0].at(0)))  // is channel
	{
	} else {
	}
	/*
	 Parameters:
		<channel>
		{[+|-]|o|p|s|i|t|n|b|v}
		[<limit>]
		[<user>]
		[<ban mask>]
	*/
}
// Utils
bool Server::validNickname(std::string nickname) {
	if (nickname.empty() || isdigit(nickname[0])) {
		return false;
	}

	std::string disallowedChars = FORBIDDEN_USER_CHARS;
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

	std::string prefixes = CHANNEL_PREFIX;

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
