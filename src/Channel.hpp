#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>

#include "Client.hpp"

#define MAX_CHANNEL_NAME 200
#define CHANNEL_PREFIX "&#"
#define CHANNEL_INVALID_CHARS " ,"
#define USER_CHANNEL_LIMIT 10

class Channel {
	public:
	Channel(void);
	Channel(std::string const &nickname, std::string const &username,
			std::string const &host);
	Channel(Channel const &ref);
	~Channel(void);
	Channel &operator=(Channel const &ref);

	private:
	std::string	 nickname;
	unsigned int modes;
	std::string	 username;
	std::string	 host;
	Client		 channel_operator;
};

#endif
