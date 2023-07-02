#include "Utils.hpp"

std::vector<std::string> splitString(std::string& source) {
	std::vector<std::string> tokens;
	std::string				 singleToken;

	// TODO: Prefix are neccesary?
	if (source[0] == ':') {	 // Prefix
		std::istringstream sourceStream(source);
		std::getline(sourceStream, singleToken, ' ');

		removeNewlines(singleToken);

		tokens.push_back(singleToken);
		source = source.substr(1);
	}

	// TODO: Find a better solution later
	size_t colon = source.find(":");
	if (colon != std::string::npos) {
		std::string beforeColon = source.substr(0, colon);
		std::string afterColon	= source.substr(colon);

		std::istringstream beforeColonSteam(beforeColon);

		while (std::getline(beforeColonSteam, singleToken, ' ')) {
			removeNewlines(singleToken);
			tokens.push_back(singleToken);
		}

		removeNewlines(afterColon);
		tokens.push_back(afterColon);
	} else {
		std::istringstream sourceStream(source);

		while (std::getline(sourceStream, singleToken, ' ')) {
			removeNewlines(singleToken);
			tokens.push_back(singleToken);
		}
	}

	return tokens;
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

void removeNewlines(std::string& str) {
	str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
	str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}
