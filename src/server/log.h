#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

typedef enum {
	LogLevelCrit,
	LogLevelErr,
	LogLevelWarning,
	LogLevelNotice,
	LogLevelInfo,
	LogLevelDebug,
} LogLevel;

void log(LogLevel level, const char *format, ...);
void vlog(LogLevel level, const char *format, va_list ap);

#endif