#include <sstream>

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
		if (find((*it)->getOperators().begin(), (*it)->getOperators().end(),
				 ch) != (*it)->getOperators().end()) {
			ss << "@" << (*it)->getName();
			ss << " ";
		} else {
			ss << (*it)->getName();
			ss << " ";
		}
	}

	ss << "\r\n";

	ss << ":localhost 319 " << c->getNickname();
	ss << " " << ch->getNickname();
	ss << " localhost";
	ss << ":Your host is localhost, running version 0.1";
	ss << "\r\n";

	ss << ":localhost 317 " << c->getNickname();
	ss << " " << ch->getNickname();
	ss << " 12345678";
	ss << " 98765432";
	ss << "\r\n";

	ss << ":localhost 317 " << c->getNickname();
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

// Returns the complete mode information for a given channel
std::string Server::usermodeis(Channel &ch, Client *cli, std::string modeStr) {
	std::stringstream ss;

	ss << ":" << cli->getNickname();
	ss << " MODE";
	ss << " " << ch.getName();
	ss << " " << modeStr;
	ss << "\r\n";
	return ss.str();
}
// this function is used for MODE without flags
std::string Server::channelmodeis(pollfd p, std::string channel) {
	// 324	RPL_CHANNELMODEIS		"<channel> <mode> <mode params>"
	// :hcduller!~hcduller@Rizon-1585D411.dsl.telesp.net.br MODE #semsenhahc +ti
	// nick!user@host

	std::stringstream ss;
	Client		   *cl = &clients[p.fd];

	Channel *ch = &(getChannelByName(channel)->second);

	ss << ":localhost 324";
	ss << " " << cl->getNickname();
	ss << " " << ch->getName();
	ss << " " << ch->getStrModes();
	ss << "\r\n";
	// return ":localhost MODE #semsenha +ti \r\n";
	return ss.str();
}