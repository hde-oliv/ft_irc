#ifndef UTILS_HPP
#define UTILS_HPP

#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define P_CONTINUE 0
#define P_EXIT 1

#define RESET "\033[0m"
#define BLACK "\033[0;30m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define WHITE "\033[0;37m"

typedef struct s_command {
	std::string				 prefix;
	std::string				 cmd;
	std::vector<std::string> args;
} Command;

void		panic(std::string caller, std::string msg, int mode);
std::string getDatetime(void);
void		replaceString(std::string &subject, std::string &search,
						  std::string &replace);
Command		stringToCommand(std::string source);
std::string toIrcUpperCase(std::string s);
void		strip(std::string &str);
#endif

std::string toValidChannelName(std::string rawName);
