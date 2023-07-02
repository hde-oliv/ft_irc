#include <csignal>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Server.hpp"
#include "Utils.hpp"

bool g_online = true;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		g_online = false;
	}
}

void validateInput(int argc, char *argv[]) {
	if (argc != 3) {
		panic("main::validateInput", "Invalid number of arguments", P_EXIT);
	}

	std::string password(argv[1]);
	// TODO: Password validation

	int port = std::atoi(argv[2]);

	if (port <= 0) {
		panic("main::validateInput", "Invalid port", P_EXIT);
	}
}

int main(int argc, char *argv[]) {
	std::signal(SIGINT, signalHandler);
	try {
		validateInput(argc, argv);
		Server srv(argv[1], std::atoi(argv[2]));
		srv.startServer();
	} catch (std::exception &e) {
		return 1;
	}
}
