#ifndef SYSCALL_H
#define SYSCALL_H

#include "stdarg.h"
#include "stddef.h"
#include "stdint.h"

void sys_init(void (*syscall)(void *, uint32_t, ...), void *syscallData);

void sys_exit(uint32_t status);

uint32_t sys_fork(void);
uint32_t sys_getpid(void);

void *sys_alloc(void *ptr, size_t size);

void sys_exec(const char *path, uint32_t argc, char **argv);

uint32_t sys_getcwd(char *buf, uint32_t size);

#endif
