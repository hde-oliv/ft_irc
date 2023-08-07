#include "ChatBot.hpp"

ChatBot::ChatBot(){};
ChatBot::~ChatBot(){};

void ChatBot::answer(Client *issuer, std::string msg) {
	unsigned int	 &state = contexts[issuer];
	std::stringstream ss;
	(void)msg;

	ss << ":chatbot!ChatBot@localhost PRIVMSG ";
	ss << issuer->getNickname();
	ss << " ";
	switch (state) {
		case 0:
			ss << wellcome();
			break;
		default:
			ss << "Opção não reconhecida, tente novamente.";
			break;
	}
	ss << "\r\n";
	issuer->setSendData(ss.str());
};

std::string ChatBot::wellcome() {
	std::string msg;

	msg = "Digite uma das opções a seguir:\r";
	msg += "1 - Elevar-se a operador do servidor";
	msg += "2 - Listar usuarios conectados";
	msg += "3 - Desligar IRC server";

	return msg;
};
// std::string ChatBot::operatorStart(Client *issuer, std::string input){};
// std::string ChatBot::operatorProc(Client *issuer, std::string input){};