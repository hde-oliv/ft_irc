#include <sstream>

#include "Channel.hpp"
#include "Server.hpp"

std::string Server::needmoreparams(pollfd p, std::string command) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 461 " << c->getNickname();
	ss << " " << command << " :Not enough parameters";
	ss << "\r\n";

	return ss.str();
}

std::string Server::alreadyregistered(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 462 " << c->getNickname();
	ss << " :You may not reregister";
	ss << "\r\n";

	return ss.str();
}

std::string Server::passwdmismatch(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 464 " << c->getNickname();
	ss << " :Password incorrect";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nonicknamegiven(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 431 " << c->getNickname();
	ss << " :No nickname given";
	ss << "\r\n";

	return ss.str();
}

std::string Server::erroneusnickname(pollfd p, std::string nickname) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 432 " << c->getNickname();
	ss << " " << nickname << " :Erroneus nickname";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nicknameinuse(pollfd p, std::string nickname) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 433 " << c->getNickname();
	ss << " " << nickname << " :Nickname is already in use";
	ss << "\r\n";

	return ss.str();
}
std::string Server::welcome(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	// RPL_WELCOME 001
	ss << ":localhost 001 " << c->getNickname();
	ss << " :Welcome to the FT_IRC server " << c->getNickname();
	ss << "\r\n";

	// RPL_YOURHOST 002
	ss << ":localhost 002 " << c->getNickname();
	ss << " :Your host is localhost, running version 0.1";
	ss << "\r\n";

	// RPL_CREATED 003
	ss << ":localhost 003 " << c->getNickname();
	ss << " :This server was created " << creationDatetime;
	ss << "\r\n";

	// RPL_MYINFO 004
	ss << ":localhost 004 " << c->getNickname();
	ss << " localhost 0.1 iowstRb- biklmnopstvrRcCNuMTD";
	ss << "\r\n";

	return ss.str();
}

std::string Server::motd(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 375 " << c->getNickname();
	ss << " :- localhost Message of the day -";
	ss << "\r\n";

	ss << ":localhost 372 " << c->getNickname();
	ss << " :- Welcome to FT_IRC!";
	ss << "\r\n";

	ss << ":localhost 376 " << c->getNickname();
	ss << " :End of MOTD command.";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nooperhost(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 491 " << c->getNickname();
	ss << " :No O-lines for your host";
	ss << "\r\n";

	return ss.str();
}

std::string Server::youreoper(pollfd p) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 381 " << c->getNickname();
	ss << " :You are now an IRC operator";
	ss << "\r\n";

	return ss.str();
}

std::string Server::unknowncommand(pollfd p, std::string command) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 421 " << c->getNickname();
	ss << " " << command;
	ss << " :Unknown command";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nosuchchannel(pollfd p, std::string name) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 403 " << c->getNickname();
	ss << " " << name;
	ss << " :No such channel";
	ss << "\r\n";

	return ss.str();
}

std::string Server::topic(pollfd p, Channel *ch) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 332 " << c->getNickname();
	ss << " " << ch->getName();
	ss << " :" << ch->getTopic();
	ss << "\r\n";

	return ss.str();
}

std::string Server::notopic(pollfd p, Channel *ch) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 331 " << c->getNickname();
	ss << " " << ch->getName();
	ss << " :No topic is set";
	ss << "\r\n";

	return ss.str();
}

std::string Server::namreply(pollfd p, Channel *ch) {
	std::stringstream ss;

	Client *c		= &clients[p.fd];
	Client *creator = ch->getCreator();

	ss << ":localhost 353 " << c->getNickname();
	ss << " = :" << ch->getName() << " ";
	if (creator) {
		ss << "!" << creator->getNickname() << " ";
	}
	std::map<Client *, unsigned int>::iterator cli = ch->getClients().begin();
	while (cli != ch->getClients().end()) {
		if (creator != NULL) {
			if (cli->first == creator) {
				cli++;
				continue;
			}
		}
		if (cli->second & USER_OPERATOR) {
			ss << "@";
		}
		ss << (*cli->first).getNickname() << " ";
		cli++;
	}
	ss << "\r\n";

	// TODO: Consider the RFC for 366 implementation later

	ss << ":localhost 366 " << c->getNickname();
	ss << " " << ch->getName();
	ss << " :End of NAMES list";
	ss << "\r\n";

	return ss.str();
}

std::string Server::whoisreply(pollfd p, Client *ch) {
	Client		   *c = &clients[p.fd];
	std::stringstream ss;

	ss << ":localhost 311 " << c->getNickname();
	ss << " " << ch->getNickname();
	ss << " " << ch->getUsername();
	ss << " localhost";
	if (ch->getOp()) {
		ss << " *";
	}  // TODO: Check if is voiced
	ss << " " << ch->getRealname();
	ss << "\r\n";

	ss << ":localhost 319 " << c->getNickname();
	ss << " " << ch->getNickname();
	ss << " :";

	std::vector<Channel *>::iterator it = ch->getChannels().begin();

	for (; it != ch->getChannels().end(); it++) {
		std::map<Client *, uint> clients = (*it)->getClients();
		uint					 modes	 = clients[ch];

		if (modes & USER_OPERATOR) {
			ss << "@" << (*it)->getName();
			ss << " ";
		} else {
			ss << (*it)->getName();
			ss << " ";
		}
	}

	ss << "\r\n";

	ss << ":localhost 312 " << c->getNickname();
	ss << " " << ch->getNickname();
	ss << " localhost";
	ss << " :Your host is localhost, running version 0.1";
	ss << "\r\n";

	ss << ":localhost 317 " << c->getNickname();
	ss << " " << ch->getNickname();
	ss << " 12345678";
	ss << " 98765432";
	ss << "\r\n";

	ss << ":localhost 318 " << c->getNickname();
	ss << " " << ch->getNickname();
	ss << " :End of WHOIS list";
	ss << "\r\n";

	return ss.str();
}

std::string Server::whoreply(pollfd p, Channel *ch) {
	std::stringstream						   ss;
	Client									*c	   = &clients[p.fd];
	std::map<Client *, unsigned int>		   clients = ch->getClients();
	std::map<Client *, unsigned int>::iterator cli	   = clients.begin();

	// NOTE: Who is the worse command to implement
	// check later if it can be skipped

	for (; cli != clients.end(); cli++) {
		ss << ":localhost 352 " << c->getNickname();
		ss << " " << ch->getName();
		ss << " " << (*cli->first).getUsername();
		ss << " " << (*cli->first).getHostname();
		ss << " " << (*cli->first).getServername();
		ss << " " << (*cli->first).getNickname();

		if ((*cli->first).getOp()) {
			ss << " G";
		} else {
			ss << " H";
		}
		if (cli->second & USER_OPERATOR) {
			ss << "@";
		}
		// TODO: Check for voiced

		ss << " :0";
		ss << " " << (*cli->first).getRealname();
		ss << "\r\n";
	}

	ss << ":localhost 315";
	ss << " " << c->getNickname();
	ss << " " << ch->getName();
	ss << " :End of WHO list";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nosuchserver(pollfd p, std::string name) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 402";
	ss << " " << c->getNickname();
	ss << " " << name;
	ss << " :No such server";
	ss << "\r\n";

	return ss.str();
}

std::string Server::badchannelkey(pollfd p, std::string channel) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 475";
	ss << " " << c->getNickname();
	ss << " " << channel;
	ss << " :Cannot join channel (+)";
	ss << "\r\n";

	return ss.str();
}

std::string Server::unknownmode(pollfd p, char c) {
	std::stringstream ss;
	Client		   *cl = &clients[p.fd];

	ss << ":localhost 472";
	ss << " " << cl->getNickname();
	ss << " " << c;
	ss << " :is unknown mode char to me";
	ss << "\r\n";

	return ss.str();
}
// 221	RPL_UMODEIS				"<user mode string>"

// Returns modes changed in the channel made by a client with MODE command
std::string Server::usermodeis(Channel &ch, Client *cli, std::string modeStr) {
	std::stringstream ss;

	ss << ":" << cli->getNickname();
	ss << " MODE";
	ss << " " << ch.getName();
	ss << " " << modeStr;
	ss << "\r\n";
	return ss.str();
}
// Returns usermodes changed with MODE command
// Called by user with /MODE <nickname> '+'|'-'{i|s|w|o}
std::string Server::usermodeis(Client *cli, std::string modeStr) {
	std::stringstream ss;
	// :hdeoliv!~hcduller@Rizon-6F5C18E9.dsl.telesp.net.br MODE hdeoliv :+i
	// :hcduller!~hcduller@Rizon-6F5C18E9.dsl.telesp.net.br MODE hcduller :-i

	ss << ":" << cli->getNickname();
	ss << " MODE 221";
	ss << " :" << modeStr;
	ss << "\r\n";
	return ss.str();
}
// Returns user modes in relation to the server.
// Called by user with /MODE <nickname>
std::string Server::usermodeis(Client *cli) {
	std::stringstream ss;
	//: irc.uworld.se 221 hdeoliv +ix

	ss << ":localhost 221";
	ss << " " << cli->getNickname();
	ss << " " << cli->getModesStr();
	ss << "\r\n";
	return ss.str();
}
// This function is used for MODE without flags, returning the complete list of
// modes enabled for a given channel
std::string Server::channelmodeis(pollfd p, std::string channel) {
	Client		   *cl = &clients[p.fd];
	std::stringstream ss;

	std::map<std::string, Channel>::iterator ch_it = getChannelByName(channel);
	if (ch_it == channels.end()) {
		return nosuchchannel(p, channel);
	}
	Channel *ch = &(ch_it->second);

	ss << ":localhost 324";
	ss << " " << cl->getNickname();
	ss << " " << ch->getName();
	ss << " " << ch->getStrModes();
	ss << "\r\n";
	// return ":localhost MODE #semsenha +ti \r\n";
	return ss.str();
}

std::string Server::usersdontmatch(Client *cli) {
	std::stringstream ss;

	ss << ":localhost 502";
	ss << " " << cli->getNickname();
	ss << " :Cannot change mode for other users";
	ss << "\r\n";
	return ss.str();
};

std::string Server::unknownmodeflag(Client *cli) {
	std::stringstream ss;

	ss << ":localhost 501";
	ss << " " << cli->getNickname();
	ss << " :Unknown MODE flag";
	ss << "\r\n";
	return ss.str();
};
std::string Server::notonchannel(pollfd p, std::string name) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 442";
	ss << " " << c->getNickname();
	ss << " " << name;
	ss << " :You're not on that channel";
	ss << "\r\n";

	return ss.str();
}

std::string Server::chanoprivsneeded(pollfd p, Channel *ch) {
	std::stringstream ss;
	Client		   *c = &clients[p.fd];

	ss << ":localhost 482";
	ss << " " << c->getNickname();
	ss << " " << ch->getName();
	ss << " :You're not channel operator";
	ss << "\r\n";

	return ss.str();
}
// Success response for KICK command
std::string Server::kicksuccess(Client *cli, Channel *chan,
								std::string target) {
	std::stringstream ss;

	ss << ":" << cli->getNickname();
	ss << " KICK " << chan->getName();
	ss << " " << target;
	ss << " :" << cli->getNickname();
	ss << "\r\n";
	return ss.str();
};
// Failure response for KICK command
//	where the user was not found in the channel
std::string Server::usernotinchannel(Client *cli, Channel *chan,
									 std::string target) {
	std::stringstream ss;

	ss << ":localhost 441";
	ss << " " << cli->getNickname();
	ss << " " << target;
	ss << " " << chan->getName();
	ss << " :They aren't on that";
	ss << "\r\n";

	return ss.str();
}
// Failure response for KICK command
//  where the issuer has no operator privileges
std::string Server::chanoprivsneeded(Client *issuer, Channel *chan) {
	std::stringstream ss;

	ss << ":localhost 482";
	ss << " " << issuer->getNickname();
	ss << " " << chan->getName();
	ss << " :You're not channel operator";
	ss << "\r\n";

	return ss.str();
}

std::string Server::norecipient(pollfd p, std::string name) {
	Client		   *cl = &clients[p.fd];
	std::stringstream ss;

	ss << ":localhost 411";
	ss << " " << cl->getNickname();
	ss << " :No recipient given (";
	ss << name << ")";
	ss << "\r\n";

	return ss.str();
}

std::string Server::notexttosend(pollfd p) {
	Client		   *cl = &clients[p.fd];
	std::stringstream ss;

	ss << ":localhost 412";
	ss << " " << cl->getNickname();
	ss << " :No text to send";
	ss << "\r\n";

	return ss.str();
}

std::string Server::nosuchnick(pollfd p, std::string name) {
	Client		   *cl = &clients[p.fd];
	std::stringstream ss;

	ss << ":localhost 401";
	ss << " " << cl->getNickname();
	ss << " " << name;
	ss << " :No such nick/channel";
	ss << "\r\n";

	return ss.str();
}

// Sends :<serverName> 341 <issuer> <target> <channel>
std::string Server::inviting(Client *issuer, Client *target, Channel *ch) {
	std::stringstream ss;

	ss << ":localhost 341";
	ss << " " << issuer->getNickname();
	ss << " " << target->getNickname();
	ss << " " << ch->getName();
	ss << "\r\n";

	return ss.str();
}

// :<issuer> INVITE <target> :<channel>
std::string Server::inviterrpl(Client *issuer, Client *target, Channel *ch) {
	std::stringstream ss;

	ss << ":" << issuer->getNickname();
	ss << " INVITE";
	ss << " " << target->getNickname();
	ss << " :" << ch->getName();
	ss << "\r\n";

	return ss.str();
}
// Returns
// :<host> 473 <issuer> <channel> :Cannot join channel (+i)
std::string Server::inviteonlychan(Client *issuer, Channel *ch) {
	std::stringstream ss;

	ss << ":localhost 473";
	ss << " " << issuer->getNickname();
	ss << " " << ch->getName();
	ss << " :Cannot join channel (+i)";
	ss << "\r\n";

	return ss.str();
}
