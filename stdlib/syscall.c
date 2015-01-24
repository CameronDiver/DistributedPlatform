#include "stddef.h"
#include "syscall.h"

void (*sys_syscall)(void *, uint32_t, ...)=NULL;
void *sys_data=NULL;

void sys_init(void (*syscall)(void *, uint32_t id, ...), void *syscallData)
{
	sys_syscall=syscall;
	sys_data=syscallData;
}

void sys_exit(uint32_t status)
{
	(*sys_syscall)(sys_data, 0, status);
}

uint32_t sys_fork(void)
{
	uint32_t ret;
	(*sys_syscall)(sys_data, 1, &ret);
	return ret;
}

uint32_t sys_getpid(void)
{
	uint32_t ret;
	(*sys_syscall)(sys_data, 2, &ret);
	return ret;
}
