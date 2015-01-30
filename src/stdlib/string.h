#ifndef STRING_H
#define STRING_H

#include "stddef.h"

void *Dmemset(void *s, int c, size_t n);

size_t Dstrlen(const char *s);


void *Dmemcpy(void *dest, const void *src, size_t n);
void *Dmemmove(void *dest, const void *src, size_t n);

#endif
