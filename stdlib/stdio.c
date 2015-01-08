#include "stdio.h"

int printf(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int ret=vprintf(format, ap);
	va_end(ap);
	return ret;
}

int fprintf(FILE *stream, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int ret=vfprintf(stream, format, ap);
	va_end(ap);
	return ret;
}

int sprintf(char *str, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int ret=vsprintf(str, format, ap);
	va_end(ap);
	return ret;
}

int snprintf(char *str, size_t size, const char *format, ...) {
	// TODO: snprintf
	return 0;
}

int vprintf(const char *format, va_list ap) {
	// TODO: vprintf
	return 0;
}

int vfprintf(FILE *stream, const char *format, va_list ap) {
	// TODO: vfprintf
	return 0;
}

int vsprintf(char *str, const char *format, va_list ap) {
	// TODO: vsprintf
	return 0;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
	// TODO: vsnprintf
	return 0;
}
