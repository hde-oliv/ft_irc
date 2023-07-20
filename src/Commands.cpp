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

	c->setOp(true);
	c->setSendData(youreoper(p));
	// TODO: Send MODE +o
}

void Server::privmsg(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	if (t.args.size() < 2) {
		c->setSendData(needmoreparams(p, "PRIVMSG"));
		return;
	}

	Channel *ch = &channels[toIrcUpperCase(t.args[0])];

	ss << c->getClientPrefix();
	ss << " PRIVMSG";
	ss << " ";
	ss << t.args[0];
	ss << " ";
	ss << t.args[1];
	ss << "\r\n";

	ch->broadcast(c, ss.str(), false);
}

void Server::join(pollfd p, Command &t) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	// TODO: Check later for ERR_BADCHANMASK
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

	// NOTE: Check if client sent a password and if its incorrect or
	// the server has a password and the client didnt provide one
	// TODO: Check if these responses are correct
	if (sentPassword) {
		if (!ch->evalPassword(t.args[1]))
			return c->setSendData(badchannelkey(p, ch->getName()));
	} else {
		if (!ch->evalPassword(""))
			return c->setSendData(badchannelkey(p, ch->getName()));
	}

	ch->addClient(c);

	// TODO: after implementing BANS, this line should check for bans!
	ss << c->getClientPrefix();
	ss << " JOIN :";
	ss << t.args[0];
	ss << "\r\n";

	ch->broadcast(c, ss.str(), true);

	if (ch->getTopic() != "") {
		c->setSendData(topic(p, ch));
	} else {
		c->setSendData(notopic(p, ch));
	}

	c->setSendData(namreply(p, ch));

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
		return;
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

	// TODO: Check later what other servers respond when the Channel parameter
	// does not exist
	c->setSendData(nosuchserver(p, t.args[0]));
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

void Server::channelMode(pollfd p, Command &t) {
	Client *c = &clients[p.fd];

	std::string								 toggleMode		= "psitnmk";
	std::string								 cmdsWithParams = "olbv";
	std::string								 cmdPrefix		= "+-";
	std::map<std::string, Channel>::iterator it = getChannelByName(t.args[0]);

	if (it == channels.end()) {
		return (c->setSendData(nosuchchannel(p, "MODE")));
	}

	Channel								   &ch = it->second;
	std::map<Client *, unsigned int>::iterator cli =
		ch.getClientByNick(c->getNickname());

	if (!(cli->second & USER_OPERATOR)) return;

	if (cmdPrefix.find(t.args[1][0]) == std::string::npos)
		return (c->setSendData(unknownmode(p, t.args[0][1])));

	bool on = t.args[1][0] == '+';
	t.args[1].erase(0, 1);

	std::set<char> chFlags;
	char		   usrFlag = 0;
	while (t.args[1].size() > 0) {
		if (cmdsWithParams.find(t.args[1][0]) != std::string::npos) {
			chFlags.insert(t.args[1][0]);
		} else {
			usrFlag = t.args[1][0];
		}
		t.args[1].erase(0, 1);
	}

	// handle toggles
	std::set<char>::iterator modeIt;
	for (std::size_t i; i < toggleMode.size(); i++) {
		modeIt = chFlags.find(toggleMode.at(i));
		if (modeIt != chFlags.end()) {
			ch.toggleMode(*modeIt, on);
		}
	}

	// handle non-toggle
	switch (usrFlag) {
		case 'l':
			/* limit channel */
			break;
		case 'o':
			/* set Operator */
			break;
		case 'b':
			/* ban mask */
			break;
		case 'v':
			/* mude */
			break;

		default:
			break;
	}

	// uses first argument after modes t.args[2]

	/*
	Parameters:
		<channel>
		o l b v
		p s i t n m k
		[<limit>]
		[<user>]
		[<ban mask>]
	std::vector<char> cmdModes;
	while (t.args[1].size() > 0) {	// do not execute USER cmds
		cmdModes.push_back(t.args[1][0]);
		t.args[0].erase(0, 1);
		if (toggleMode.find(cmdModes.back()) != std::string::npos) {
			ch.toggleMode(cmdModes.back(), on);
			cmdModes.pop_back();
		}
	}
	*/
}

void Server::mode(pollfd p, Command &t) {
	Client	   *c		  = &clients[p.fd];
	std::string ch_prefix = CHANNEL_PREFIX;

	// identify if command applies to channel or client
	if (t.args.size() < 2)
		return c->setSendData(
			needmoreparams(p, "MODE"));	 // ERR_NEEDMOREPARAMS 461

	if (ch_prefix.find(t.args[0].at(0))) {
		if (evalChanMode(p, t.args)) {
			channelMode(p, t);
		}
	} else {
		/*
			Parameters: <nickname> {[+|-]|i|w|s|o}
		*/
		return;
	}
	/*
		resps:
		324	RPL_CHANNELMODEIS		"<channel> <mode> <mode params>"
		367	RPL_BANLIST				"<channel> <banid>"
		368	RPL_ENDOFBANLIST		"<channel> :End of channel ban list"
			- When listing the active 'bans' for a
				given channel, a server is required to send the list back using
				the RPL_BANLIST and RPL_ENDOFBANLIST messages. A separate
				RPL_BANLIST is sent for each active banid.  After the banids
				have been listed (or if none present) a RPL_ENDOFBANLIST must be
				sent.

		221	RPL_UMODEIS				"<user mode string>"
			- To answer a query about a client's own mode,
			RPL_UMODEIS is sent back.

		442	ERR_NOTONCHANNEL		"<channel> :You're not on that channel"
		461	ERR_NEEDMOREPARAMS		"<command> :Not enough parameters"
		482	ERR_CHANOPRIVSNEEDED	"<channel> :You're not channel operator"
		472	ERR_UNKNOWNMODE			"<char> :is unknown mode char to me"
		502	ERR_USERSDONTMATCH		":Cant change mode for other users"
		401	ERR_NOSUCHNICK			"<nickname> :No such nick/channel"
		467	ERR_KEYSET				"<channel> :Channel key already set"
		403	ERR_NOSUCHCHANNEL		"<channel name> :No such channel"
		501	ERR_UMODEUNKNOWNFLAG	":Unknown MODE flag"
			- Returned by the server to indicate that a MODE
				  message was sent with a nickname parameter and that
				  the a mode flag sent was not recognized.
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
