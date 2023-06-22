#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Logger.hpp"
#include "Server.hpp"
#include "Utils.hpp"

void validateInput(int argc, char *argv[]) {
	if (argc != 3) {
		panic("validateInput(main:17)", "Invalid number of arguments");
	}

	std::string password(argv[1]);
	// TODO: Password validation

	int port = std::atoi(argv[2]);

	if (port <= 0) {
		panic("validateInput(main:39)", "Invalid port");
	}
}

int main(int argc, char *argv[]) {
	try {
		validateInput(argc, argv);
		Server srv(argv[1], std::atoi(argv[2]));
		srv.startServer();
	} catch (std::exception &e) {
		return 1;
	}
}
