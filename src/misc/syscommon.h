#ifndef SYSCOMMON_H
#define SYSCOMMON_H

#include <stdint.h>

typedef int (*SysCommonProcMain)(int, const char **);

typedef struct
{
	//TODO: Do we need a version number of sorts (to ensure stdlib version of this struct matches)?
	int32_t argc;
	const char **argv; // TODO: Static assert that sizeof(char)==1?
	SysCommonProcMain main;
	void (*syscall)(void *, uint32_t, ...);
	void *syscallData;
	char **environ;
} SysCommonProcInfo;

typedef enum {
	SysCommonSysCallExit,
	SysCommonSysCallFork,
	SysCommonSysCallGetPid,
	SysCommonSysCallAlloc,
	SysCommonSysCallExec,
	SysCommonSysCallGetCwd,
} SysCommonSysCall;

#endif