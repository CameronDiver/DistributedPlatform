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

void *Dmemcpy(void *dest, const void *src, size_t n) {
	return Dmemmove(dest, src, n);
}

void *Dmemmove(void *dest, const void *src, size_t n) {
	char *d=(char *)dest;
	const char *s=(const char *)s;
	while(n-->0)
		*d++=*s++;
	return dest;
}
