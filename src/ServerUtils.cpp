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

	std::string toggleMode	   = "psitnmk";
	std::string cmdsWithParams = "olbv";
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
		cli->setSendData(
			unknownmode(p, '!'));  // ! means and attempt to change channel and
								   // users simultaniously
		return false;
	}

	if (togglePair.first) return true;

	if (paramPair.first) {
		if (paramPair.second > 1 || args.size() < 3) {
			cli->setSendData(
				needmoreparams(p, "MODE"));	 // This error code might need to be
											 // changed
			return false;
		}
		return true;
	}
	cli->setSendData(needmoreparams(p, "MODE"));
	return false;
	// args 0 = composition of '+' | '-' and { p | s | i | t | n | m | k } | one
	// of [ o | l | b | v ] args 1 is the parameter needed by o | l | b | v
}