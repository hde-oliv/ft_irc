#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#define MAX_NICKNAME 9

class Client {
	public:
	Client(void);
	Client(std::string const &nickname, std::string const &username,
		   std::string const &host);
	Client(Client const &ref);
	~Client(void);
	Client &operator=(Client const &ref);

	protected:
	std::string nickname;
	std::string username;
	std::string host;
};

class Operator : public Client {
	public:
	Operator(void);
	Operator(std::string const &nickname, std::string const &username,
			 std::string const &host);
	Operator(Operator const &ref);
	~Operator(void);
	Operator &operator=(Operator const &ref);

	protected:
	std::string nickname;
	std::string username;
	std::string host;
};

// TODO: Consider this later
// Because of IRC's scandanavian origin, the characters {}| are
// considered to be the lower case equivalents of the characters []\,
// respectively. This is a critical issue when determining the
// equivalence of two nicknames.

#endif
