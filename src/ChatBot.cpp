#include "ChatBot.hpp"

#include "Server.hpp"

ChatBot::ChatBot(Server &srv) : server(srv){};
ChatBot::~ChatBot(){};

extern bool g_online;

void ChatBot::answer(Client *issuer, std::string msg) {
	unsigned int	 &state = contexts[issuer];
	std::stringstream ss;

	msg.erase(0, 1);
	ss << ":chatbot!ChatBot@localhost PRIVMSG ";
	ss << issuer->getNickname();
	ss << " ";
	switch (state) {
		case CB_INIT:
			ss << wellcome();
			state = CB_MAINANS;
			break;
		case CB_MAINANS:
			ss << mainAnswer(issuer, state, msg);
			break;
		case CB_WAITPSW:
			ss << waitPsw(issuer, state, msg);
			state = CB_MAINANS;
			break;
		default:
			ss << "Opção não reconhecida, tente novamente.";
			break;
	}
	ss << "\r\n";
	send(issuer->getFd(), ss.str().c_str(), ss.str().size(), 0);
	// issuer->setSendData(ss.str());
};

std::string ChatBot::wellcome() {
	std::string msg;

	msg = "\rDigite uma das opções a seguir:\r";
	msg += "1 - Elevar-se a operador do servidor\r";
	msg += "2 - Listar usuarios conectados\r";
	msg += "3 - Desligar IRC server";

	return msg;
};

std::string ChatBot::mainAnswer(Client *issuer, unsigned int &state,
								std::string input) {
	std::string answer;
	if (input == "1") {
		answer = "\rDigite a senha de operador\r";
		answer += "Digite 1 para retornar ao menu inicial.";
		state = CB_WAITPSW;
	} else if (input == "2") {
		answer = listUsers(issuer);
		answer += wellcome();
	} else if (input == "3") {
		answer = shutdownServer(issuer);
	} else {
		answer = "\rOpção não reconhecida.\r";
		answer += wellcome();
	}
	return answer;
}

std::string ChatBot::waitPsw(Client *issuer, unsigned int &state,
							 std::string input) {
	std::string answer;
	if (input == "1") {
		state  = CB_MAINANS;
		answer = wellcome();
	} else {
		if (input != OPER_PASS) {
			answer = "\rSenha incorreta, tente novamente ou digite 1";
		} else {
			issuer->setOp(true);
			answer = "\rVocê foi elevado a operador do servidor\r";
			answer += wellcome();
			state = CB_MAINANS;
		}
	}
	return answer;
}

std::string ChatBot::listUsers(Client *issuer) {
	bool isOperator = issuer->getOp();

	std::vector<std::pair<std::string, bool> > users;
	users = server.listClients();

	std::string usuarios = "\rUsuarios online:\r";
	std::vector<std::pair<std::string, bool> >::iterator it;
	it = users.begin();
	while (it != users.end()) {
		if (it->second) {
			usuarios += it->first;
			usuarios += "\r";
		} else {
			if (isOperator) {
				usuarios += it->first;
				usuarios += "(invisivel)\r";
			}
		}
		it++;
	}
	usuarios += "Fim da lista de usuarios\r";
	return usuarios;
};

std::string ChatBot::shutdownServer(Client *issuer) {
	if (issuer->getOp()) {
		g_online = false;
		return "IRC Server sera desligado.";
	} else {
		return "Você precisa ser um operador do servidor para utilizar este "
			   "comando.";
	}
};