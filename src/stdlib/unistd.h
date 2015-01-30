#ifndef UNISTD_H
#define UNISTD_H

#include "stdarg.h"
#include "types.h"

pid_t Dfork(void);
pid_t Dgetpid(void);

int Dexecl(const char *path, const char *arg, ...);
int Dexecv(const char *path, char *const argv[]);

#endif
