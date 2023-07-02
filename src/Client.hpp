#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#define MAX_NICKNAME 9
#define BUFFER_SIZE 512

class Client {
	public:
	Client(void);
	~Client(void);

	void setHost(std::string host);
	void setFd(int fd);
	void setReadData(std::string data);
	void setSendData(std::string data);
	void setServerPassword(std::string password);
	void resetData();

	std::string getNickname() const;
	std::string getUsername() const;
	std::string getHost() const;
	std::string getReadData() const;
	std::string getSendData() const;
	std::string getServerPassword() const;

	private:
	int			fd;
	std::string nickname;
	std::string username;
	std::string host;
	std::string readData;
	std::string sendData;
	std::string serverPassword;
};

// TODO: Consider this later
// Because of IRC's scandanavian origin, the characters {}| are
// considered to be the lower case equivalents of the characters []\,
// respectively. This is a critical issue when determining the
// equivalence of two nicknames.

// They MUST NOT contain any of the following characters: space (' ', 0x20),
// comma (',', 0x2C), asterisk ('*', 0x2A), question mark ('?', 0x3F),
// exclamation mark ('!', 0x21), at sign ('@', 0x40).
// They MUST NOT start with any of the following characters: dollar ('$', 0x24),
// colon (':', 0x3A). They MUST NOT start with a character listed as a channel
// type prefix. They SHOULD NOT contain any dot character ('.', 0x2E).
#endif
