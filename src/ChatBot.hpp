#pragma once
#include <map>
#include <sstream>
#include <string>

#include "Client.hpp"

#define CB_INIT 0
#define CB_MAINANS 1
#define CB_WAITPSW 2

class Server;

class ChatBot {
	public:
	~ChatBot();
	ChatBot(Server &srv);
	void answer(Client *issuer, std::string msg);

	private:
	std::map<Client *, unsigned int> contexts;

	Server	   &server;
	std::string wellcome();

	std::string mainAnswer(Client *issuer, unsigned int &state,
						   std::string input);
	std::string waitPsw(Client *issuer, unsigned int &state, std::string input);
	std::string listUsers(Client *issuer);
	std::string shutdownServer(Client *issuer);
};