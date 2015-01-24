#ifndef STDLIB_H
#define STDLIB_H

#include "stddef.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int Dabs(int x);
void Dexit(int status);

void *Dmalloc(size_t size);
void Dfree(void *ptr);
void *Dcalloc(size_t nmemb, size_t size);
void *Drealloc(void *ptr, size_t size);

#endif
