#include "string.h"

void *memset(void *s, int c, size_t n)
{
	unsigned char *d=s;
	while (n-->0)
		*d++=(unsigned char)c;
	return s;
}
