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
	// Following system calls roughly match those found in Linux.
	SysCommonSysCallExit=1,
	SysCommonSysCallFork=2,
	SysCommonSysCallRead=3,
	SysCommonSysCallWrite=4,
	SysCommonSysCallOpen=5,
	SysCommonSysCallClose=6,
	SysCommonSysCallExec=11,
	SysCommonSysCallChDir=12,
	SysCommonSysCallGetPid=20,
	SysCommonSysCallGetCwd=183,

	// Unique calls.
	SysCommonSysCallAlloc,
} SysCommonSysCall;

#endif