#include "Utils.hpp"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> splitString(std::string& source) {
	std::vector<std::string> tokens;
	std::string				 single_token;
	std::istringstream		 sourceStream(source);
	while (std::getline(sourceStream, single_token, ' ')) {
		tokens.push_back(single_token);
	}
	return tokens;
}
void panic(std::string caller, std::string msg) {
	std::cerr << "Exception on " << caller << ": " << msg << std::endl;

	perror("ERRNO");  // TODO: Remove this later

	throw std::exception();
}
