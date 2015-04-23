#ifndef SYS_H
#define SYS_H

#include "stdarg.h"
#include "stdint.h"

//typedef uint32_t pid_t; // TODO: How to stop <stdlib.h> including <sys/types.h>?

pid_t fork(void);
pid_t getpid(void);

int execl(const char *path, const char *arg, ...);
int execv(const char *path, char *const argv[]);

char *getcwd(char *buf, size_t size);

#endif
