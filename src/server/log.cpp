#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <time.h>

#include "log.h"

const char *logStr[]={"CRITICAL", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

void log(LogLevel level, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vlog(level, format, ap);
	va_end(ap);
}

void vlog(LogLevel level, const char *format, va_list ap) {
	// Open file for appending.
	FILE *file=fopen("./distlog", "a");
	if (file==NULL)
		return;

	// Create timestamp.
	time_t timeRaw;
	struct tm *timeInfo;
	time(&timeRaw);
	timeInfo=localtime(&timeRaw);
	char str[128];
	strftime(str, 512, "%G-%m-%d %H:%M:%S", timeInfo);

	fprintf(file, "%s DistServer %s - ", str, logStr[level]);
	vfprintf(file, format, ap);

	fclose(file);
}