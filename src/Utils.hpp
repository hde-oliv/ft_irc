#ifndef UTILS_HPP
#define UTILS_HPP

#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define P_CONTINUE 0
#define P_EXIT 1

void					 panic(std::string caller, std::string msg, int mode);
std::vector<std::string> splitString(std::string& source);
#endif
