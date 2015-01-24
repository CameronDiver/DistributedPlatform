#include "stdint.h"
#include "stdlib.h"
#include "string.h"
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

void *Dmalloc(size_t size)
{
	return Drealloc(NULL, size);
}

void Dfree(void *ptr)
{
	Drealloc(ptr, 0);
}

void *Dcalloc(size_t nmemb, size_t size)
{
	void *ptr=Drealloc(NULL, nmemb*size);
	if (ptr==NULL)
		return NULL;
	memset(ptr, 0, nmemb*size);
	return ptr;
}

void *Drealloc(void *ptr, size_t size)
{
	return sys_alloc(ptr, size);
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
