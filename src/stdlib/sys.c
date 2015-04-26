#include "stdlib.h"
#include "string.h"
#include "sys.h"
#include "syscall.h"

pid_t __wrap_fork(void) {
	return sys_fork();
}

pid_t __wrap_getpid(void) {
	return sys_getpid();
}

int __wrap_execl(const char *path, const char *arg, ...) {
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
	char **argv=malloc(sizeof(char *)*(argc+1));
	if (argv==NULL)
		goto error;

	// Copy first argument.
	size_t arg0Size=strlen(arg)+1;
	argv[0]=malloc(arg0Size);
	if (argv[0]==NULL)
		goto error;
	memcpy(argv[0], arg, arg0Size);

	// Copy other arguments.
	unsigned int i;
	for(i=1;(nextArg=(const char *)va_arg(ap, const char *))!=NULL;++i)
	{
		size_t argSize=strlen(nextArg)+1;
		argv[i]=malloc(argSize);
		if (argv[i]==NULL)
			goto error;
		memcpy(argv[i], nextArg, argSize);
	}
	va_end(ap);

	// Null terminated list.
	argv[argc]=NULL;
	
	// Call execv to do the rest of the work.
	int ret=execv(path, argv);

	// Clean up.
	for(i=0;argv[i]!=NULL;++i)
		free(argv[i]);
	free(argv);

	return ret;

	error:
	if (argv!=NULL) {
		for(i=0;argv[i]!=NULL;++i)
			free(argv[i]);
		free(argv);
	}
	va_end(ap);
	return -1;
}

int __wrap_execv(const char *path, char *const gargv[]) {
	// Find argc.
	uint32_t argc;
	for(argc=0;gargv[argc]!=NULL;++argc) ;
	
	// Allocate argv.
	char **argv=malloc(sizeof(char *)*argc);
	if (argv==NULL)
		return -1;
	
	// Copy each arg.
	unsigned int i;
	for(i=0;i<argc;++i)
	{
		size_t argSize=strlen(gargv[i])+1;
		argv[i]=malloc(argSize);
		if (argv[i]==NULL)
		{
			unsigned int j;
			for(j=0;j<i;++j)
				free(argv[j]);
			free(argv);
			return -1;
		}
		memcpy(argv[i], gargv[i], argSize);
	}
	
	// Use system call to replace process.
	sys_exec(path, argc, argv);
	
	return -1; // sys_exec only returns if error.
}

int __wrap_chdir(const char *path) {
	return sys_chdir(path);
}

char *__wrap_getcwd(char *buf, size_t size) {
	// Check buffer.
	if (buf==NULL) {
		// TODO: Set errno=EFAULT;
		return NULL;
	}

	// Check size;
	if (size==0) {
		// TODO: Set errno=EINVAL;
		return NULL;
	}

	// Use system call.
	size_t trueSize=sys_getcwd(buf, size);

	// Check for range error.
	if (trueSize>size) {
		// TODO: Set errno=ERANGE;
		return NULL;
	}

	// Success.
	return buf;
}

ssize_t __wrap_read(int fd, void *buf, size_t count) {
	return sys_read(fd, buf, count);
}

ssize_t __wrap_write(int fd, const void *buf, size_t count) {
	return sys_write(fd, buf, count);
}

int __wrap_open(const char *pathname, int flags, mode_t mode) {
	return sys_open(pathname, flags, mode);
}

int __wrap_close(int fd) {
	return sys_close(fd);
}