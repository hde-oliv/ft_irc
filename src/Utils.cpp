#include "Utils.hpp"

#include <cstdio>
#include <iostream>

void *ft_memset(void *b, int c, size_t len) {
	void		  *p;
	unsigned char *s;

	p = b;
	s = (unsigned char *)b;
	while (len-- > 0) *s++ = (unsigned char)c;
	return (p);
}

void panic(std::string caller, std::string msg) {
	std::cerr << "Exception on " << caller << ": " << msg << std::endl;

	perror("ERRNO");  // TODO: Remove this later

	throw std::exception();
}
