#include "Utils.hpp"

Command stringToCommand(std::string source) {
	Command c;

	std::string token;

	if (source[0] == ':') {
		std::istringstream sourceStream(source);
		std::getline(sourceStream, c.prefix, ' ');
		strip(c.prefix);
		source = source.substr(source.find(c.prefix) + c.prefix.size());
	}

	strip(source);
	std::istringstream sourceStream(source);
	std::getline(sourceStream, c.cmd, ' ');
	source = source.substr(source.find(c.cmd) + c.cmd.size());

	// TODO: Find a better solution later
	size_t colon = source.find(":");
	if (colon != std::string::npos) {
		std::string beforeColon = source.substr(0, colon);
		std::string afterColon	= source.substr(colon);

		std::istringstream beforeColonSteam(beforeColon);

		while (std::getline(beforeColonSteam, token, ' ')) {
			strip(token);
			if (token.size() != 0) c.args.push_back(token);
		}

		strip(afterColon);
		c.args.push_back(afterColon);
	} else {
		std::istringstream sourceStream(source);

		while (std::getline(sourceStream, token, ' ')) {
			strip(token);
			if (token.size() != 0) c.args.push_back(token);
		}
	}

	return c;
}
void panic(std::string caller, std::string msg, int mode) {
	std::cerr << RED << "Exception on " << caller << RESET << ": " << msg
			  << std::endl;

	// TODO: Remove this later
	std::cerr << BLUE << "ERRNO: " << RESET << std::strerror(errno)
			  << std ::endl;

	if (mode == P_EXIT) {
		throw std::exception();
	}
}

std::string getDatetime() {
	std::time_t currentTime = std::time(NULL);
	std::tm*	localTime	= std::localtime(&currentTime);

	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);

	return std::string(buffer);
}

void replaceString(std::string& subject, const std::string& search,
				   const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

std::string toUppercase(std::string s) {
	// NOTE: Check this later
	transform(s.begin(), s.end(), s.begin(), ::toupper);
	replaceString(s, "{", "[");
	replaceString(s, "}", "]");
	replaceString(s, "|", "\\");

	return s;
}

void strip(std::string& str) {
	str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
	str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

	std::size_t start = 0;
	std::size_t end	  = str.length();

	while (start < end && std::isspace(str[start])) {
		start++;
	}

	while (end > start && std::isspace(str[end - 1])) {
		end--;
	}

	str = str.substr(start, end - start);
}
