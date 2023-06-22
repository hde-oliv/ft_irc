#include "Utils.hpp"

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
	std::cerr << caller << " : " << msg << std::endl;

	throw std::exception();
}
