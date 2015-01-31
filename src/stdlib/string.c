#include "string.h"

char *strtokSave;

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

char *Dstrchr(const char *s, int c) {
	for(;*s!=c;++s)
		if (*s=='\0')
			return NULL;
	return (char *)s;
}

size_t Dstrspn(const char *s, const char *accept) {
  size_t n;
  const char* p;
  for(n=0; *s; s++, n++) {
    for(p=accept; *p && *p != *s; p++)
      ;
    if (!*p)
      break;
  }
  return n;
}

size_t Dstrcspn(const char *s, const char *reject) {
	size_t count;
	for(count=0;Dstrchr(reject, *s)==0;++count)
		++s;
	return count;
}

char *Dstrtok(char *str, const char *delim) {
	return Dstrtok_r(str, delim, &strtokSave);
}

char *Dstrtok_r(char *str, const char *delim, char **saveptr) {
	char *ret;
	
	// Continuing previous search?
	if (str==NULL)
		str=*saveptr;
	
	// Find next delim.
	str+=Dstrspn(str, delim);
	if (*str=='\0')
		return NULL;
	ret=str;
	
	str+=Dstrcspn(str, delim);
	if(*str)
		*str++='\0';
	*saveptr=str;
	
	return ret;
}
