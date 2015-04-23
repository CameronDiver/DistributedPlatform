#include "stddef.h"
#include "syscall.h"

#include "../misc/syscommon.h"

void (*sys_syscall)(void *, uint32_t, ...)=NULL;
void *sys_data=NULL;

void sys_init(void (*syscall)(void *, uint32_t id, ...), void *syscallData) {
	sys_syscall=syscall;
	sys_data=syscallData;
}

void sys_exit(uint32_t status) {
	(*sys_syscall)(sys_data, SysCommonSysCallExit, status);
}

uint32_t sys_fork(void) {
	uint32_t ret;
	(*sys_syscall)(sys_data, SysCommonSysCallFork, &ret);
	return ret;
}

uint32_t sys_getpid(void) {
	uint32_t ret;
	(*sys_syscall)(sys_data, SysCommonSysCallGetPid, &ret);
	return ret;
}

void *sys_alloc(void *ptr, size_t size) {
	void *ret;
	(*sys_syscall)(sys_data, SysCommonSysCallAlloc, &ret, ptr, size);
	return ret;
}

void sys_exec(const char *path, uint32_t argc, char **argv) {
	(*sys_syscall)(sys_data, SysCommonSysCallExec, path, argc, argv);
}

int32_t sys_chdir(const char *path) {
	int32_t ret;
	(*sys_syscall)(sys_data, SysCommonSysCallChDir, &ret, path);
	return ret;
}

uint32_t sys_getcwd(char *buf, uint32_t size) {
	uint32_t ret;
	(*sys_syscall)(sys_data, SysCommonSysCallGetCwd, &ret, buf, size);
	return ret;
}