#include "Utils.hpp"

void *ft_memset(void *b, int c, size_t len) {
	void		  *p;
	unsigned char *s;

	p = b;
	s = (unsigned char *)b;
	while (len-- > 0) *s++ = (unsigned char)c;
	return (p);
}
