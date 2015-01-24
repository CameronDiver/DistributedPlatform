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

#endif
