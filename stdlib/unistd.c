#include "syscall.h"
#include "unistd.h"

pid_t Dfork(void)
{
	return sys_fork();
}

pid_t Dgetpid(void)
{
	return sys_getpid();
}
