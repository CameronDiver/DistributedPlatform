#ifndef SYS_H
#define SYS_H

#include "stdarg.h"
#include "stdint.h"

//typedef uint32_t pid_t; // TODO: How to stop <stdlib.h> including <sys/types.h>?

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2

pid_t fork(void);
pid_t getpid(void);

int execl(const char *path, const char *arg, ...);
int execv(const char *path, char *const argv[]);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int open(const char *pathname, int flags, mode_t mode);
int close(int fd);

#endif
