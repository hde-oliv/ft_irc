#include <sys/poll.h>

#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include "Channel.hpp"
#include "Server.hpp"
#include "Utils.hpp"

void Server::pass(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() == 0) {
		c->setSendData(needmoreparams(p, "PASS"));
		return;
	} else if (c->getRegistration() & PASS_FLAG) {
		c->setSendData(alreadyregistered(p));
		return;
	} else if (t.args[0].substr(1) != password) {
		c->setSendData(passwdmismatch(p));
		return;
	}

	c->setKnowPassword(true);
	c->setRegistration(PASS_FLAG);
}

void Server::user(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() < 4) {
		c->setSendData(needmoreparams(p, "USER"));
		return;
	} else if (c->getRegistration() & USER_FLAG) {
		c->setSendData(alreadyregistered(p));
		return;
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
		c->setSendData(nonicknamegiven(p));
		return;
	} else if (!validNickname(t.args[0])) {
		c->setSendData(erroneusnickname(p, t.args[0]));
		return;
	} else if (nicknameAlreadyExists(t.args[0])) {
		c->setSendData(nicknameinuse(p, t.args[0]));
		return;
	}
	c->setNickname(t.args[0]);
	c->setRegistration(NICK_FLAG);
}

void Server::oper(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() < 2) {
		c->setSendData(needmoreparams(p, t.cmd));
		return;
	} else if (t.args[1] != OPER_PASS) {
		c->setSendData(passwdmismatch(p));
		return;
	} else if (t.args[0] != OPER_USER) {
		c->setSendData(nooperhost(p));
		return;
	}

	if (c->setMode('o', true)) {
		c->setSendData(youreoper(p));
	}
}

void Server::privmsg(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;
	std::string		  ch_prefix = CHANNEL_PREFIX;

	if (t.args.size() == 1) {
		if (ch_prefix.find(t.args[0].at(0)) != std::string::npos)
			return c->setSendData(norecipient(p, "PRIVMSG"));
		else
			return c->setSendData(notexttosend(p));
	}

	if (t.args.size() < 2) {
		c->setSendData(needmoreparams(p, "PRIVMSG"));
		return;
	}

	std::string botname = CHATBOTNAME;
	if (toIrcUpperCase(t.args[0]) == botname) {
		return bot.answer(c, t.args[1]);
	}
	ss << c->getClientPrefix();
	ss << " PRIVMSG";
	ss << " ";
	ss << t.args[0];
	ss << " ";
	ss << t.args[1];
	ss << "\r\n";

	if (ch_prefix.find(t.args[0].at(0)) != std::string::npos) {
		std::map<std::string, Channel>::iterator it =
			channels.find(toIrcUpperCase(t.args[0]));

		if (it == channels.end())
			return c->setSendData(nosuchnick(p, t.args[0]));
		else
			return it->second.broadcast(c, ss.str(), false);
	} else {
		std::map<int, Client>::iterator it = clients.begin();

		for (; it != clients.end(); it++) {
			if (it->second.getNickname() == t.args[0]) {
				return it->second.setSendData(ss.str());
			}
		}
		if (it == clients.end()) {
			return c->setSendData(nosuchnick(p, t.args[0]));
		}
	}
}

void Server::join(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	if (t.args.size() < 1) {
		c->setSendData(needmoreparams(p, "JOIN"));
		return;
	}

	if (!validChannelName(t.args[0])) {
		c->setSendData(nosuchchannel(p, t.args[0]));
		return;
	}

	bool sentPassword = t.args.size() > 1;

	if (sentPassword) {
		bool v = validatePassword(t.args[1]);
		if (!v) {
			c->setSendData(needmoreparams(p, "JOIN"));
			return;
		}
	}

	Channel *ch = &channels[toIrcUpperCase(t.args[0])];

	if (!ch->isInitialized()) {
		if (sentPassword) {
			ch->initialize(t.args[0], t.args[1], c);
		} else {
			ch->initialize(t.args[0], c);
		}
	}

	std::set<char> &chModes = ch->getMode();
	if (chModes.find('l') != chModes.end()) {
		if (ch->getClients().size() >= ch->getUserLimit()) {
			return c->setSendData(channelisfull(c, ch));
		}
	}
	if (chModes.find('i') != chModes.end()) {
		if (!ch->isInvited(c->getNickname())) {
			return c->setSendData(inviteonlychan(c, ch));
		}
		ch->removeInvited(c->getNickname());
		return successfulJoin(c, ch);
	}

	// NOTE: Check if client sent a password and if its incorrect or
	// the server has a password and the client didnt provide one
	if (sentPassword) {
		if (!ch->evalPassword(t.args[1]))
			return c->setSendData(badchannelkey(p, ch->getName()));
	} else {
		if (!ch->evalPassword(""))
			return c->setSendData(badchannelkey(p, ch->getName()));
	}
	successfulJoin(c, ch);
}

void Server::successfulJoin(Client *cli, Channel *ch) {
	pollfd p = pollFds[cli->getFd()];

	std::stringstream ss;
	ch->addClient(cli);
	cli->addChannel(ch);
	ss << cli->getClientPrefix();
	ss << " JOIN :";
	ss << ch->getName();
	ss << "\r\n";

	ch->broadcast(cli, ss.str(), true);

	if (ch->getTopic() != "") {
		cli->setSendData(topic(p, ch));
	} else {
		cli->setSendData(notopic(p, ch));
	}

	cli->setSendData(namreply(p, ch, true));
};

void Server::who(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	// NOTE: Not defined in RFC
	if (t.args.size() < 1) {
		c->setSendData(needmoreparams(p, "WHO"));
		return;
	}

	std::map<std::string, Channel>::iterator it;

	for (it = channels.begin(); it != channels.end(); it++) {
		if (it->first == toIrcUpperCase(t.args[0])) {
			return c->setSendData(whoreply(p, &(it->second)));
		}
	}

	c->setSendData(nosuchnick(p, t.args[0]));
}

void Server::topic(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	if (t.args.size() < 1) {
		c->setSendData(needmoreparams(p, "TOPIC"));
		return;
	}

	Channel *ch = &channels[toIrcUpperCase(t.args[0])];

	if (t.args.size() == 1) {
		if (ch->getTopic() != "") {
			return c->setSendData(topic(p, ch));
		} else {
			return c->setSendData(notopic(p, ch));
		}
	}

	std::map<Client *, uint>		   cls = ch->getClients();
	std::map<Client *, uint>::iterator it  = cls.find(c);
	std::set<char>					   md  = ch->getMode();

	if (it == cls.end()) {
		return c->setSendData(notonchannel(p, ch->getName()));
	}

	if (find(md.begin(), md.end(), 't') != md.end()) {
		std::map<Client *, uint> cls = ch->getClients();

		std::map<Client *, uint>::iterator it = cls.find(c);

		if (it != cls.end() && it->second & USER_OPERATOR) {
			ch->setTopic(t.args[1]);
			return ch->broadcast(c, topic(p, ch), true);
		} else {
			return c->setSendData(chanoprivsneeded(p, ch));
		}
	} else {
		ch->setTopic(t.args[1]);
		return c->setSendData(topic(p, ch));
	}
}

void Server::whois(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	// NOTE: Not defined in RFC
	if (t.args.size() < 1) {
		c->setSendData(needmoreparams(p, "WHOIS"));
		return;
	}

	std::map<int, Client>::iterator it;

	for (it = clients.begin(); it != clients.end(); it++) {
		if ((it->second).getNickname() == t.args[0]) {
			c->setSendData(whoisreply(p, &(it->second)));
			return;
		}
	}

	c->setSendData(nosuchnick(p, t.args[0]));
}

void Server::quit(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	ss << ":" << c->getNickname();
	if (t.args.size()) {
		ss << " QUIT " << t.args[0];
	} else {
		ss << " QUIT :Gone to have lunch";
	}
	ss << "\r\n";

	// NOTE: Actually is not broadcasted
	// broadcastMessage(p, ss.str());
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

void Server::part(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	if (t.args.size() < 1) {
		return c->setSendData(needmoreparams(p, "PART"));
	}

	std::map<std::string, Channel>::iterator it =
		channels.find(toIrcUpperCase(t.args[0]));

	if (it == channels.end()) {
		return c->setSendData(nosuchchannel(p, t.args[0]));
	}

	std::vector<Channel *> chs = c->getChannels();

	std::vector<Channel *>::iterator chanIt = chs.begin();
	while (chanIt != chs.end()) {
		if (toIrcUpperCase((*chanIt)->getName()) == toIrcUpperCase(t.args[0])) {
			if (t.args.size() != 2)
				return removeClientFromChannel(c, *chanIt,
											   ":" + c->getNickname());
			else
				return removeClientFromChannel(c, *chanIt, t.args[1]);
		}
		chanIt++;
	}
	return c->setSendData(notonchannel(p, t.args[0]));
}

void Server::notice(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	if (t.args.size() < 2) {
		return;
	}

	ss << c->getClientPrefix();
	ss << " NOTICE";
	ss << " ";
	ss << t.args[0];
	ss << " ";
	ss << t.args[1];
	ss << "\r\n";

	std::string ch_prefix = CHANNEL_PREFIX;
	if (ch_prefix.find(t.args[0].at(0)) != std::string::npos) {
		Channel *ch = &channels[toIrcUpperCase(t.args[0])];
		ch->broadcast(c, ss.str(), false);
		return;
	} else {
		std::map<int, Client>::iterator it = clients.begin();

		for (; it != clients.end(); it++) {
			if (it->second.getNickname() == t.args[0]) {
				it->second.setSendData(ss.str());
				return;
			}
		}
	}
}

void Server::channelMode(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	std::string toggleMode	   = "it";
	std::string cmdsWithParams = "lko";
	std::string cmdPrefix	   = "+-";

	std::map<std::string, Channel>::iterator it = getChannelByName(t.args[0]);

	if (it == channels.end()) {
		return (c->setSendData(nosuchchannel(p, "MODE")));
	}
	Channel								   &ch = it->second;
	std::map<Client *, unsigned int>::iterator cli =
		ch.getClientByNick(c->getNickname());

	if (!(cli->second & USER_OPERATOR)) {
		c->setSendData(chanoprivsneeded(p, &ch));
		return;
	}

	if (cmdPrefix.find(t.args[1][0]) == std::string::npos)
		// todo, the second argument might be a nickname, which will trigger an
		// usermode inside this channel answer
		/*
		MODE #semsenha hcduller
		:irc.uworld.se 472 hcduller d :is unknown mode char to me
		:irc.uworld.se 349 hcduller #semsenha :End of Channel Exception List

		*/
		return (c->setSendData(unknownmode(p, t.args[0][1])));

	bool		on = t.args[1][0] == '+';
	std::string modesChanged;
	modesChanged.insert(modesChanged.begin(), t.args[1][0]);
	t.args[1].erase(0, 1);

	std::set<char> chFlags;
	char		   usrFlag = 0;
	while (t.args[1].size() > 0) {
		if (cmdsWithParams.find(t.args[1][0]) != std::string::npos) {
			usrFlag = t.args[1][0];
		} else {
			chFlags.insert(t.args[1][0]);
		}
		t.args[1].erase(0, 1);
	}

	// handle toggles
	std::set<char>::iterator modeIt;
	for (std::size_t i = 0; i < toggleMode.size(); i++) {
		modeIt = chFlags.find(toggleMode.at(i));
		if (modeIt != chFlags.end()) {
			if (ch.toggleMode(*modeIt, on)) {
				modesChanged += *modeIt;
			}
		}
	}

	// handle non-toggle
	std::stringstream ss;
	int				  lim = 0;
	switch (usrFlag) {
		case 'l':  // this is not affected by + -
			ss << t.args[2];
			if (!(ss >> lim) || lim <= 0) {
				return (c->setSendData(needmoreparams(p, "MODE")));
			}
			ch.setUserLimit(lim);
			modesChanged += "l " + t.args[2];
			break;
		case 'o':
			if (ch.setOperator(t.args[2], on)) {
				modesChanged += "o " + t.args[2];
			} else {
				return;
			}
			break;
		case 'v':
			ch.setSpeaker(t.args[2], on);
			break;
		case 'k':
			ch.toggleMode('k', on);
			if (on) {
				ch.setPassword(t.args[2]);
				modesChanged += "k " + t.args[2];
			} else {
				ch.removePassword();
				modesChanged += "k *";
			}
			break;
		default:
			break;
	}

	if (modesChanged.size() > 1)
		return ch.broadcast(c, usermodeis(ch, c, modesChanged), true);
}
void Server::userMode(pollfd p, Command &t) {
	std::string inputModes = t.args[1];
	std::string changes;

	Client *c = &clients[p.fd];

	bool on = inputModes.at(0) == '+' ? true : false;
	changes.append(1, inputModes.at(0));

	std::set<char> flags;
	size_t		   i = 1;
	while (i < inputModes.size()) {
		flags.insert(inputModes.at(i));
		i++;
	}

	std::set<char>::iterator it = flags.begin();
	while (it != flags.end()) {	 // this prevents users from elevating
								 // themselves
		if (*it == 'o' && on) {
			it++;
			continue;
		}
		if (c->setMode(*it, on)) {
			changes.append(1, *it);
		}
		it++;
	}
	if (changes.length() == 1) return;
	return c->setSendData(usermodeis(c, changes));
};

void Server::mode(pollfd p, Command &t) {
	Client	   *c		  = &clients[p.fd];
	std::string ch_prefix = CHANNEL_PREFIX;

	// identify if command applies to channel or client
	if (t.args.size() < 2) {
		if (ch_prefix.find(t.args[0].at(0)) != std::string::npos)
			return c->setSendData(channelmodeis(p, t.args[0]));
		else {
			if (toIrcUpperCase(c->getNickname()) == toIrcUpperCase(t.args[0]))
				return c->setSendData(usermodeis(c));
			else
				return c->setSendData(usersdontmatch(c));
		}
	}

	if (ch_prefix.find(t.args[0].at(0)) != std::string::npos) {
		if (evalChanMode(p, t.args)) {
			return channelMode(p, t);
		}
	} else {
		if (evalUserMode(p, t.args)) {
			return userMode(p, t);
		}
		return;
	}
}

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

	std::string disallowedNick = CHATBOTNAME;
	if (uppercaseNickname == disallowedNick) {
		return true;
	}

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

void Server::kick(pollfd p, Command &t) {
	Client *c = &clients[p.fd];
	if (t.args.size() < 2) {
		return c->setSendData(needmoreparams(p, "KICK"));
	}
	Channel *ch = NULL;

	std::map<std::string, Channel>::iterator it = getChannelByName(t.args[0]);
	if (it != channels.end()) {
		ch = &it->second;
	}
	if (ch == NULL) {
		return c->setSendData(nosuchchannel(p, t.args[0]));
	}

	std::map<Client *, unsigned int>::iterator target;
	target = ch->getClientByNick(t.args[1]);
	if (target == ch->getClients().end()) {
		return c->setSendData(usernotinchannel(c, ch, t.args[1]));
	}

	std::map<Client *, unsigned int>::iterator issuer;
	issuer = ch->getClientByNick(c->getNickname());
	if (issuer == ch->getClients().end()) {
		return c->setSendData(notonchannel(p, ch->getName()));
	}
	if (!(issuer->second & USER_OPERATOR))
		return c->setSendData(chanoprivsneeded(issuer->first, ch));
	ch->removeClient(target->first);
	ch->broadcast(c, kicksuccess(c, ch, target->first->getNickname()), true);
	return;
};

void Server::invite(pollfd p, Command &t) {
	Client *issuer = &clients[p.fd];
	if (t.args.size() < 2) {
		return issuer->setSendData(needmoreparams(p, "INVITE"));
	}

	std::map<std::string, Channel>::iterator ch_it;
	ch_it = this->getChannelByName(t.args[1]);
	if (ch_it == channels.end()) {
		return issuer->setSendData(nosuchnick(p, t.args[1]));
	}
	Channel *chan = &ch_it->second;

	Client *target = getClientByNick(t.args[0]);
	if (target == NULL) {
		return issuer->setSendData(nosuchnick(p, t.args[0]));
	}

	std::map<Client *, unsigned int>::iterator issuer_it;
	issuer_it = chan->getClientByNick(issuer->getNickname());
	if (issuer_it == chan->getClients().end()) {
		return issuer->setSendData(notonchannel(p, issuer->getNickname()));
	}

	bool isOper = issuer_it->second & USER_OPERATOR;

	std::set<char> &chanModes = chan->getMode();

	if (chanModes.find('i') != chanModes.end() && !isOper) {
		return issuer->setSendData(chanoprivsneeded(issuer, chan));
	}

	chan->addInvite(target->getNickname());
	issuer->setSendData(inviting(issuer, target, chan));
	return target->setSendData(inviterrpl(issuer, target, chan));
}
