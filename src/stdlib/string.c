#include "string.h"

void *Dmemset(void *s, int c, size_t n) {
	unsigned char *d=s;
	while (n-->0)
		*d++=(unsigned char)c;
	return s;
}

size_t Dstrlen(const char *s)	{
	size_t len;
	for(len=0;*s!='\0';++len) ++s;
	return len;
}
