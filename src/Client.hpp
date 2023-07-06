#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#define MAX_NICKNAME 9
#define BUFFER_SIZE 512

#define PASS_FLAG 2
#define USER_FLAG 4
#define NICK_FLAG 8

#define OPER_USER "foo"
#define OPER_PASS "bar"

class Client {
	public:
	Client(void);
	~Client(void);

	void setFd(int fd);
	void setReadData(std::string data);
	void setSendData(std::string data);
	void setHostname(std::string name);
	void setServername(std::string name);
	void setUsername(std::string name);
	void setRealname(std::string name);
	void setNickname(std::string name);
	void setRegistration(int flag);
	void setOp(bool value);
	void setKnowPassword(bool value);
	void setToDisconnect(bool value);
	void setWelcome(bool value);
	void resetAllData();
	void resetReadData();
	void resetSendData(int len);

	int			getFd() const;
	int			getRegistration() const;
	bool		getWelcome() const;
	bool		getKnowPassword() const;
	bool		getOp() const;
	bool		getToDisconnect() const;
	std::string getNickname() const;
	std::string getUsername() const;
	std::string getHostname() const;
	std::string getServername() const;
	std::string getRealname() const;
	std::string getReadData() const;
	std::string getSendData() const;

	private:
	int			fd;
	int			registration;  // PASS(2) - USER(4) - NICK(8)
	bool		welcome;	   // If the welcome message was sent
	bool		knowPassword;  // If the client knows the server password
	bool		op;			   // If the client is a Server Operator
	bool		toDisconnect;
	std::string nickname;
	std::string username;
	std::string hostname;
	std::string servername;
	std::string realname;
	std::string readData;
	std::string sendData;
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
