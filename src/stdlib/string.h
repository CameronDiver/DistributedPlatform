#ifndef STRING_H
#define STRING_H

#include "stddef.h"

void *Dmemset(void *s, int c, size_t n);

size_t Dstrlen(const char *s);

char *Dstrcpy(char *dest, const char *src);
char *Dstrncpy(char *dest, const char *src, size_t n);

void *Dmemcpy(void *dest, const void *src, size_t n);
void *Dmemmove(void *dest, const void *src, size_t n);

char *Dstrchr(const char *s, int c);

size_t Dstrspn(const char *s, const char *accept);
size_t Dstrcspn(const char *s, const char *reject);

char *Dstrtok(char *str, const char *delim);
char *Dstrtok_r(char *str, const char *delim, char **saveptr);

#endif
