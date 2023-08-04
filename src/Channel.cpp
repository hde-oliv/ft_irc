#include "Channel.hpp"

#include <stdexcept>

#include "Utils.hpp"

Channel::Channel() {
	host		= "localhost";
	creator		= NULL;
	userLimit	= USER_CHANNEL_LIMIT;
	initialized = false;
}

Channel::~Channel() {}

std::string Channel::getName() const { return name; };
std::string Channel::getTopic() const { return topic; }

void Channel::setInitialized(bool value) { initialized = value; }
bool Channel::isInitialized() { return initialized; }

std::map<Client *, unsigned int> &Channel::getClients() { return clients; }

void Channel::setName(std::string name) { this->name = name; }
void Channel::setTopic(std::string topic) { this->topic = topic; };

void Channel::addClient(Client *c) { clients.insert(std::make_pair(c, 0)); }

void Channel::setPassword(std::string password) { this->password = password; }
void Channel::removePassword() { this->password = ""; };

void Channel::removeClient(Client *c) {
	if (creator == c) {
		creator = NULL;
	}
	clients.erase(c);
	if (clients.size() > 0) {
		asureOperator();
	}
}

void Channel::broadcast(Client *sender, std::string message, bool toSend) {
	std::map<Client *, unsigned int>::iterator it = clients.begin();

	for (; it != clients.end(); it++) {
		if (it->first == sender && toSend) {
			(*it->first).setSendData(message);
		} else if (it->first != sender) {
			(*it->first).setSendData(message);
		}
	}
}

void Channel::setCreator(Client *c) { creator = c; };

Client *Channel::getCreator() { return creator; };

bool Channel::toggleMode(char mode, bool on) {
	std::size_t before = modes.size();
	if (on)
		modes.insert(mode);
	else
		modes.erase(mode);
	return !(before == modes.size());
}

std::set<char> &Channel::getMode() { return modes; }

void Channel::initialize(std::string name, std::string password, Client *op) {
	this->name	   = name;
	this->password = password;
	this->clients.insert(std::make_pair(op, USER_OPERATOR));
	this->initialized = true;
	this->modes.insert('t');
}

void Channel::initialize(std::string name, Client *op) {
	this->name = name;
	this->clients.insert(std::make_pair(op, USER_OPERATOR));
	this->initialized = true;
	this->modes.insert('t');
}

void		 Channel::setUserLimit(unsigned int limit) { userLimit = limit; };
unsigned int Channel::getUserLimit() const { return userLimit; };

bool Channel::setOperator(std::string clientNickname, bool newValue) {
	std::map<Client *, unsigned int>::iterator it =
		getClientByNick(clientNickname);
	if (it != clients.end()) {
		if (newValue)
			it->second |= USER_OPERATOR;
		else
			it->second &= (~USER_OPERATOR);
		return true;
	}
	return (false);
};

void Channel::setSpeaker(std::string clientNickname, bool newValue) {
	std::map<Client *, unsigned int>::iterator it =
		getClientByNick(clientNickname);
	if (it != clients.end()) {
		if (newValue)
			it->second |= USER_SPEAKER;
		else
			it->second &= (~USER_SPEAKER);
	}
};
bool Channel::evalPassword(std::string psw) { return (password == psw); }

std::map<Client *, unsigned int>::iterator Channel::getClientByNick(
	std::string clientNickname) {
	std::map<Client *, unsigned int>::iterator it = clients.begin();
	while (it != clients.end()) {
		if (it->first->getNickname() == clientNickname) return (it);
		it++;
	}
	return (it);
};

// Returns <channel> <mode>
std::string Channel::getStrModes() {
	std::string modeStr = "";

	if (modes.size() > 0) {
		modeStr += "+";
		std::set<char>::iterator it = modes.begin();
		while (it != modes.end()) {
			modeStr += (*it);
			it++;
		}
		if (modeStr.find('k') != std::string::npos) {
			modeStr.append(" ");
			modeStr.append(this->password);
		}
	}
	return (modeStr);
};
bool Channel::isInvited(std::string nickname) {
	if (invited.find(toIrcUpperCase(nickname)) != invited.end()) {
		return true;
	}
	return false;
};

void Channel::addInvite(std::string nickname) {
	invited.insert(toIrcUpperCase(nickname));
};
void Channel::removeInvited(std::string nickname) {
	invited.erase(toIrcUpperCase(nickname));
};
/*
Elevates the first user to operator when there is none in the channel.
This function requires that there is at least 1 client in it.
*/
void Channel::asureOperator() {
	std::map<Client *, unsigned int>::iterator it;
	it = clients.begin();
	while (it != clients.end()) {
		if (it->second & USER_OPERATOR) return;
		it++;
	}
	it		   = clients.begin();
	it->second = it->second | USER_OPERATOR;

	// std::stringstream ss;
	// Client		   *c = it->first;

	// ss << ":localhost 353 " << c->getNickname();
	// ss << " = :" << name << " ";
	// if (creator) {
	// 	ss << "!" << creator->getNickname() << " ";
	// }

	// std::map<Client *, unsigned int>::iterator cli = clients.begin();

	// while (cli != clients.end()) {
	// 	if (creator != NULL) {
	// 		if (cli->first == creator) {
	// 			cli++;
	// 			continue;
	// 		}
	// 	}
	// 	if (cli->second & USER_OPERATOR) {
	// 		ss << "@";
	// 	}
	// 	ss << (*cli->first).getNickname() << " ";
	// 	cli++;
	// }
	// ss << "\r\n";
	// ss << ":localhost 366 " << c->getNickname();
	// ss << " " << name;
	// ss << " :End of NAMES list";
	// ss << "\r\n";
	// broadcast(it->first, ss.str(), true);
};