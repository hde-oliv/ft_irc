#include "Utils.hpp"

std::vector<std::string> splitString(std::string& source) {
	std::vector<std::string> tokens;
	std::string				 singleToken;

	// TODO: Prefix are neccesary?
	if (source[0] == ':') {	 // Prefix
		std::istringstream sourceStream(source);
		std::getline(sourceStream, singleToken, ' ');
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
			tokens.push_back(singleToken);
		}
		tokens.push_back(afterColon);
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
