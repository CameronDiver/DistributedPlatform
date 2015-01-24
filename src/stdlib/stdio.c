#include "stdio.h"

int Dprintf(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int ret=Dvprintf(format, ap);
	va_end(ap);
	return ret;
}

int Dfprintf(FILE *stream, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int ret=Dvfprintf(stream, format, ap);
	va_end(ap);
	return ret;
}

int Dsprintf(char *str, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int ret=Dvsprintf(str, format, ap);
	va_end(ap);
	return ret;
}

int Dsnprintf(char *str, size_t size, const char *format, ...) {
	// TODO: snprintf
	return 0;
}

int Dvprintf(const char *format, va_list ap) {
	// TODO: vprintf
	return vprintf(format, ap);
}

int Dvfprintf(FILE *stream, const char *format, va_list ap) {
	// TODO: vfprintf
	return 0;
}

int Dvsprintf(char *str, const char *format, va_list ap) {
	// TODO: vsprintf
	return 0;
}

int Dvsnprintf(char *str, size_t size, const char *format, va_list ap) {
	// TODO: vsnprintf
	return 0;
}
