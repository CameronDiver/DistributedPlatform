#include "stdlib.h"
#include "string.h"
#include "syscall.h"
#include "unistd.h"

pid_t Dfork(void) {
	return sys_fork();
}

pid_t Dgetpid(void) {
	return sys_getpid();
}


int Dexecl(const char *path, const char *arg, ...) {
	va_list ap;
	va_start(ap, arg);
	
	// Find argc.
	va_list ap2;
	va_copy(ap2, ap);
	const char *nextArg;
	unsigned int argc=1; // 1 not 0 for default 'arg'.
	while((nextArg=(const char *)va_arg(ap2, const char *))!=NULL)
		++argc;
	va_end(ap2);
	
	// Allocate memory for argv.
	char **argv=Dmalloc(sizeof(char *)*(argc+1));
	if (argv==NULL)
		goto error;
	
	// Copy first argument.
	size_t arg0Size=Dstrlen(arg)+1;
	argv[0]=Dmalloc(arg0Size);
	if (argv[0]==NULL)
		goto error;
	Dmemcpy(argv[0], arg, arg0Size);
	
	// Copy other arguments.
	unsigned int i;
	for(i=1;(nextArg=(const char *)va_arg(ap, const char *))!=NULL;++i)
	{
		size_t argSize=Dstrlen(nextArg)+1;
		argv[i]=Dmalloc(argSize);
		if (argv[i]==NULL)
			goto error;
		Dmemcpy(argv[i], nextArg, argSize);
	}
	va_end(ap);
	
	// Null terminated list.
	argv[argc]=NULL;
	
	// Call execv to do the rest of the work.
	int ret=Dexecv(path, argv);
	
	// Clean up.
	for(i=0;argv[i]!=NULL;++i)
		Dfree(argv[i]);
	Dfree(argv);
	
	return ret;
	
	error:
	if (argv!=NULL) {
		for(i=0;argv[i]!=NULL;++i)
			Dfree(argv[i]);
		Dfree(argv);
	}
	va_end(ap);
	return -1;
}

int Dexecv(const char *path, char *const gargv[]) {
	// Find argc.
	uint32_t argc;
	for(argc=0;gargv[argc]!=NULL;++argc) ;
	
	// Allocate argv.
	char **argv=Dmalloc(sizeof(char *)*argc);
	if (argv==NULL)
		return -1;
	
	// Copy each arg.
	unsigned int i;
	for(i=0;i<argc;++i)
	{
		size_t argSize=Dstrlen(gargv[i])+1;
		argv[i]=Dmalloc(argSize);
		if (argv[i]==NULL)
		{
			unsigned int j;
			for(j=0;j<i;++j)
				Dfree(argv[j]);
			Dfree(argv);
			return -1;
		}
		Dmemcpy(argv[i], gargv[i], argSize);
	}
	
	// Use system call to replace process.
	sys_exec(path, argc, argv);
	
	return -1; // sys_exec only returns if error.
}
