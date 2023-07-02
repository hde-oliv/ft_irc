#include "Utils.hpp"

std::vector<std::string> splitString(std::string& source) {
	std::vector<std::string> tokens;
	std::string				 single_token;
	std::istringstream		 sourceStream(source);
	while (std::getline(sourceStream, single_token, ' ')) {
		tokens.push_back(single_token);
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
