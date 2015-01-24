#ifndef STDIO_H
#define STDIO_H

#include "stdarg.h"
#include "stddef.h"

typedef struct {} FILE;

int Dprintf(const char *format, ...);
int Dfprintf(FILE *stream, const char *format, ...);
int Dsprintf(char *str, const char *format, ...);
int Dsnprintf(char *str, size_t size, const char *format, ...);
int Dvprintf(const char *format, va_list ap);
int Dvfprintf(FILE *stream, const char *format, va_list ap);
int Dvsprintf(char *str, const char *format, va_list ap);
int Dvsnprintf(char *str, size_t size, const char *format, va_list ap);

#endif
