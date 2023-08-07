#pragma once
#include <map>
#include <sstream>
#include <string>

#include "Client.hpp"

#define CB_INIT 0
#define CB_OPASIGN 0b1

class ChatBot {
	public:
	ChatBot();
	~ChatBot();
	void answer(Client *issuer, std::string msg);

	private:
	std::map<Client *, unsigned int> contexts;

	std::string wellcome();
	// std::string operatorStart(Client *issuer, std::string input);
	// std::string operatorProc(Client *issuer, std::string input);
};