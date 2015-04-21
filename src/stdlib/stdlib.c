#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "syscall.h"

char **Denviron=NULL;

typedef int (*ProcessMain)(int, const char **);

typedef struct {
	int32_t argc;
	const char **argv;
	ProcessMain main;
  void (*syscall)(void *, uint32_t, ...);
  void *syscallData;
}StdlibProcessInfo;

////////////////////////////////////////////////////////////////////////////////
// Private prototypes.
////////////////////////////////////////////////////////////////////////////////

void _start(const void *);
void _restart(const void *);

////////////////////////////////////////////////////////////////////////////////
// Public functions.
////////////////////////////////////////////////////////////////////////////////

void *__wrap_malloc(size_t size) {
	return realloc(NULL, size);
}

void __wrap_free(void *ptr) {
	realloc(ptr, 0);
}

void *__wrap_calloc(size_t nmemb, size_t size) {
	void *ptr=realloc(NULL, nmemb*size);
	if (ptr==NULL)
		return NULL;
	memset(ptr, 0, nmemb*size);
	return ptr;
}

void *__wrap_realloc(void *ptr, size_t size) {
	return sys_alloc(ptr, size);
}

void __wrap_abort(void) {
	// TODO: abort properly.
	exit(EXIT_FAILURE);
}

int __wrap_atexit(void (*func)(void)) {
	// TODO: this
	return -1;
}

void __wrap_exit(int status) {
	// TODO: Exit - call atexit functions.
	// TODO: Exit - Close (and potentially flush) streams open with stdio.
	// TODO: Exit - remove files created by tmpfile.
	sys_exit(status);
}

char *__wrap_getenv(const char* name) {
	if (Denviron==NULL)
		return NULL;

	size_t nameLen=strlen(name);
	char **ptr;
	for(ptr=Denviron;*ptr!=NULL;++ptr)
		if (strncmp(*ptr, name, nameLen)==0 && (*ptr)[nameLen]=='=')
			return (*ptr)+nameLen+1;
	return NULL;
}

int __wrap_system(const char* command) {
	// TODO: this
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Private functions.
////////////////////////////////////////////////////////////////////////////////

void _start(const void *ptr) {
	// Grab info struct.
	const StdlibProcessInfo *info=(const StdlibProcessInfo *)ptr;
	
	// Setup standard library.
	Denviron=NULL;
	sys_init(info->syscall, info->syscallData);
	
	// Run main() and exit with return value.
	exit((*info->main)(info->argc, info->argv));
}

void _restart(const void *ptr) {
	// Grab info struct.
	const StdlibProcessInfo *info=(const StdlibProcessInfo *)ptr;
	
	// Setup standard library.
	Denviron=NULL;
	sys_init(info->syscall, info->syscallData);
}