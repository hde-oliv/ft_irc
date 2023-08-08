#include <sys/poll.h>

#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include "Channel.hpp"
#include "Server.hpp"
#include "Utils.hpp"

bool Server::evalChanMode(pollfd p, std::vector<std::string> args) {
	Client *cli = &clients[p.fd];

	std::string toggleMode	   = "it";
	std::string cmdsWithParams = "lko";
	std::string cmdPrefix	   = "+-";
	std::string allModes	   = toggleMode + cmdsWithParams;

	if (args.size() < 2) {
		cli->setSendData(needmoreparams(p, "MODE"));
		return false;
	}

	std::string &modes = args[1];

	if (cmdPrefix.find(modes[0]) == std::string::npos) {
		cli->setSendData(unknowncommand(p, "MODE"));
		return false;
	}

	char prefix = modes.at(0);
	modes.erase(0, 1);

	std::pair<bool, unsigned int> togglePair = std::make_pair(false, 0);
	std::pair<bool, unsigned int> paramPair	 = std::make_pair(false, 0);
	std::string::iterator		  chars		 = modes.begin();
	while (chars != modes.end()) {
		if (toggleMode.find(chars.base()) != std::string::npos) {
			togglePair.first = true;
			togglePair.second += 1;
		} else if (cmdsWithParams.find(chars.base()) != std::string::npos) {
			paramPair.first = true;
			paramPair.second += 1;
		}
		chars++;
	}
	if (togglePair.first && paramPair.first) {
		cli->setSendData(unknownmode(p, '!'));
		return false;
	}

	if (togglePair.first) return true;

	if (paramPair.first) {
		if (args.size() < 3 && modes.at(0) == 'b') {
			return true;
		}
		if (prefix == '-' && modes.at(0) == 'k') {
			return true;
		}
		if (paramPair.second > 1 || args.size() < 3) {
			cli->setSendData(needmoreparams(p, "MODE"));
			return false;
		}
		return true;
	}
	cli->setSendData(channelmodeis(p, args[0]));
	return false;
}
bool Server::evalUserMode(pollfd p, std::vector<std::string> args) {
	Client *c = &clients[p.fd];

	if (!(toIrcUpperCase(c->getNickname()) == toIrcUpperCase(args[0]))) {
		c->setSendData(usersdontmatch(c));
		return false;
	}
	std::string allowedFlags = "iwso";
	if (args[1].at(0) != '+' && args[1].at(0) != '-') {
		c->setSendData(unknownmodeflag(c));
		return false;
	}
	if (args[1].size() < 2) {
		return false;
	}
	size_t i = 1;
	while (i < args[1].size()) {
		if (allowedFlags.find(args[1].at(i), 0) == std::string::npos) {
			c->setSendData(unknownmodeflag(c));
			return false;
		}
		i++;
	}
	return true;
}
Client *Server::getClientByNick(std::string nickname) {
	Client *c = NULL;

	std::map<int, Client>::iterator it;
	it = clients.begin();
	while (it != clients.end()) {
		if (toIrcUpperCase(nickname) ==
			toIrcUpperCase(it->second.getNickname())) {
			c = &it->second;
			break;
		}
		it++;
	}
	return c;
};