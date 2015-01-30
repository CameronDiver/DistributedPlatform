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
	char **argv=Dmalloc(sizeof(char *));
	if (argv==NULL)
		goto error;
	
	size_t arg0Size=Dstrlen(arg)+1;
	argv[0]=Dmalloc(arg0Size);
	if (argv[0]==NULL)
		goto error;
	Dmemcpy(argv[0], arg, arg0Size);
	
	// TODO: Copy other arguments into argv.
	
	int ret=Dexecv(path, argv);
	
	Dfree(argv[0]);
	Dfree(argv);
	
	return ret;
	
	error:
	if (argv!=NULL) {
		if (argv[0]!=NULL)
			Dfree(argv[0]);
		Dfree(argv);
	}
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
