#include "stdint.h"
#include "stdlib.h"
#include "syscall.h"

typedef int (*ProcessMain)(unsigned int, const char **);

typedef struct
{	
	uint16_t argc;
	const char **argv;
	ProcessMain main;
  void (*syscall)(void *, uint32_t, ...);
  void *syscallData;
}StdlibProcessInfo;

////////////////////////////////////////////////////////////////////////////////
// Private prototypes.
////////////////////////////////////////////////////////////////////////////////

void _start(const void *);

////////////////////////////////////////////////////////////////////////////////
// Public functions.
////////////////////////////////////////////////////////////////////////////////

int Dabs(int x) {
	return (x>=0 ? x : -x);
}

void Dexit(int status) {
	sys_exit(status);
}

////////////////////////////////////////////////////////////////////////////////
// Private functions.
////////////////////////////////////////////////////////////////////////////////

void _start(const void *ptr) {
	// Grab info struct.
	const StdlibProcessInfo *info=(const StdlibProcessInfo *)ptr;
	
	// Setup standard library.
	sys_init(info->syscall, info->syscallData);
	
	// Run main() and exit with return value.
	Dexit((*info->main)(info->argc, info->argv));
}
