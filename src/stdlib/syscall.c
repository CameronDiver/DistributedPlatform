#include "stddef.h"
#include "syscall.h"

void (*sys_syscall)(void *, uint32_t, ...)=NULL;
void *sys_data=NULL;

void sys_init(void (*syscall)(void *, uint32_t id, ...), void *syscallData) {
	sys_syscall=syscall;
	sys_data=syscallData;
}

void sys_exit(uint32_t status) {
	(*sys_syscall)(sys_data, 0, status);
}

uint32_t sys_fork(void) {
	uint32_t ret;
	(*sys_syscall)(sys_data, 1, &ret);
	return ret;
}

uint32_t sys_getpid(void) {
	uint32_t ret;
	(*sys_syscall)(sys_data, 2, &ret);
	return ret;
}

void *sys_alloc(void *ptr, size_t size) {
	void *ret;
	(*sys_syscall)(sys_data, 3, &ret, ptr, size);
	return ret;
}

void sys_exec(const char *path, uint32_t argc, char **argv) {
	(*sys_syscall)(sys_data, 4, path, argc, argv);
}

uint32_t sys_getcwd(char *buf, uint32_t size) {
	uint32_t ret;
	(*sys_syscall)(sys_data, 5, &ret, buf, size);
	return ret;
}