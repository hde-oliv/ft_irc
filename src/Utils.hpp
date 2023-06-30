#ifndef UTILS_HPP
#define UTILS_HPP

#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void					 panic(std::string caller, std::string msg);
std::vector<std::string> splitString(std::string& source);
#endif
