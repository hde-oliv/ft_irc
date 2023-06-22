#ifndef UTILS_HPP
#define UTILS_HPP

#include <unistd.h>

#include <string>

void *ft_memset(void *b, int c, size_t len);

void panic(std::string caller, std::string msg);

#endif
