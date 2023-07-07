#include "Utils.hpp"

void trimAll(std::string& str) {
	std::size_t start = 0;
	std::size_t end	  = str.length();

	while (start < end && str[start] == ' ') {
		start++;
	}

	while (end > start && str[end - 1] == ' ') {
		end--;
	}
	str = str.substr(start, end - start);
}

void trimStart(std::string& str) {
	std::size_t start = 0;
	std::size_t end	  = str.length();

	while (start < end && str[start] == ' ') {
		start++;
	}
	str = str.substr(start);
}

void trimEnd(std::string& str) {
	std::size_t size = str.length();
	std::size_t cut	 = 0;

	while (cut < size && std::isspace(str[size - 1 - cut])) {
		cut++;
	}
	str = str.substr(0, size - cut);
}

Command messageToCommand(std::string source) {
	Command c;

	std::string token;

	if (source[0] == ':') {
		std::istringstream sourceStream(source);
		std::getline(sourceStream, c.prefix, ' ');
		source = source.substr(source.find(c.prefix) + c.prefix.size());
	}
	trimStart(source);
	std::istringstream sourceStream(source);
	std::getline(sourceStream, c.cmd, ' ');
	source = source.substr(source.find(c.cmd) + c.cmd.size());
	trimStart(source);
	size_t colonPos = source.find(':');
	if (colonPos != std::string::npos) {
		std::string beforeColon = source.substr(0, colonPos);
		std::string afterColon	= source.substr(colonPos);

		std::istringstream beforeColonSteam(beforeColon);

		while (std::getline(beforeColonSteam, token, ' ')) {
			trimAll(token);
			if (token.size() != 0) c.args.push_back(token);
		}
		c.args.push_back(afterColon);
	} else {
		std::istringstream sourceStream(source);

		while (std::getline(sourceStream, token, ' ')) {
			trimAll(token);
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

std::string toIrcUpperCase(std::string s) {
	// NOTE: Check this later
	transform(s.begin(), s.end(), s.begin(), ::toupper);
	replaceString(s, "{", "[");
	replaceString(s, "}", "]");
	replaceString(s, "|", "\\");

	return s;
}
std::vector<std::string> splitWithToken(std::string source, char delim) {
	std::vector<std::string> subStrings;

	std::size_t start = 0;
	std::size_t end	  = source.find(delim);
	while (end != std::string::npos) {
		subStrings.push_back(source.substr(start, end - start));
		start = end + 1;
		end	  = source.find(delim, start);
	}
	subStrings.push_back(source.substr(start));
	return (subStrings);
}