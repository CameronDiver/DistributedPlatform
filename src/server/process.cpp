#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "process.h"
#include "util.h"

Process::Process(void) {
	info.argc=0;
	info.argv=NULL;
	info.main=NULL;
	info.syscall=NULL;
	info.syscallData=NULL;
	dlHandle=NULL;
	name=NULL;
	path=NULL;
	state=ProcessState::None;
}

Process::~Process(void) {
	// argc and argv.
	if (info.argv!=NULL) {
		unsigned int i;
		for(i=0;i<info.argc;++i)
		free((void *)info.argv[i]);
		free(info.argv);
		info.argv=NULL;
	}
	info.argc=0;

	// Dynamic library.
	if (dlHandle!=NULL)	{
		dlclose(dlHandle);
		dlHandle=NULL;
	}
	
	// Name.
	if (name!=NULL) {
		free(name);
		name=NULL;
	}
	
	// Path.
	if (path!=NULL) {
		free(name);
		name=NULL;
	}
	
	// Others.
	info.syscall=NULL;
	free(info.syscallData);
	info.syscallData=NULL;
	state=ProcessState::None;
}

bool Process::loadFileLocal(const char *gpath) {
	const char *pathLast;
	size_t nameSize, pathSize;
	
	// Check we are not already loaded or running.
	if (state!=ProcessState::None)
		return false;

	// Attempt to open the program as a dynamic library.
	dlHandle=dlopen(gpath, RTLD_LAZY);
	if (dlHandle==NULL)
		goto error;

	// Grab main and _start functors.
	info.main=(ProcessMain)dlsym(dlHandle, "main");
	start=(ProcessStart)dlsym(dlHandle, "_start");
	if (info.main==NULL || start==NULL)
		goto error;

	// Allocate memory for name.
	pathLast=strstrrev(gpath, "/")+1;
	nameSize=strlen(pathLast)+1;
	name=(char *)malloc(sizeof(char)*nameSize);
	if (name==NULL)
		goto error;
	memcpy((void *)name, (void *)pathLast, nameSize);
	
	// Allocate memory for path.
	pathSize=strlen(gpath)+1;
	path=(char *)malloc(sizeof(char)*pathSize);
	if (path==NULL)
		goto error;
	memcpy((void *)path, (void *)gpath, pathSize);

	// Update state.
	state=ProcessState::Loaded;
	
	// Success.
	return true;

	error:
	if (dlHandle!=NULL) {
		dlclose(dlHandle);
		dlHandle=NULL;
	}
	if (name!=NULL) {
		free(name);
		name=NULL;
	}
	if (path!=NULL) {
		free(path);
		path=NULL;
	}
	return false;
}

bool Process::loadFileFS(FS *fs, const char *path) {
	// dlopen requires a local file so ask the file system for such a file.
	char *localPath=fs->fileLocalPath(path);
	if (localPath==NULL)
		return false;
	bool ret=this->loadFileLocal(localPath);
	free(localPath);

	return ret;
}

bool Process::run(void (*syscall)(void *, uint32_t, ...), void *syscallData, bool doFork, unsigned int argc, ...) {
	va_list ap;
	va_start(ap, argc);
	bool ret=vrun(syscall, syscallData, doFork, argc, ap);
	va_end(ap);
	return ret;
}

bool Process::vrun(void (*syscall)(void *, uint32_t, ...), void *syscallData, bool doFork, unsigned int argc, va_list ap) {
	// TODO: Check argc can fit into type.

	// Ensure program is loaded but not running.
	if (state!=ProcessState::Loaded)
		return false;

	// Setup argc and argv. Do this before forking in case of error.
	info.argc=argc+1; // +1 for program name.
	info.argv=(const char **)malloc(sizeof(char *)*info.argc);
	if (info.argv==NULL)
		return false;
	unsigned int i;
	size_t size=strlen(name)+1;
	info.argv[0]=(const char *)malloc(sizeof(char)*size);
	if (info.argv[0]==NULL) {
		free((void *)info.argv);
		info.argv=NULL;
		return false;
	}
	memcpy((void *)info.argv[0], (void *)name, size);
	for(i=1;i<argc+1;++i) {
		const char *arg=(const char *)va_arg(ap, const char *);
		size_t size=strlen(arg)+1;
		info.argv[i]=(const char *)malloc(sizeof(char)*size);
		if (info.argv[i]==NULL) {
			unsigned int j;
			for(j=0;j<i;++j)
				free((void *)info.argv[j]);
			free((void *)info.argv);
			info.argv=NULL;
			return false;
		}
		memcpy((void *)info.argv[i], (void *)arg, size);
	}

	// Fork to create new process, if desired.
	pid_t pid=(doFork ? fork() : 0);
	if (pid<0)
		return false;
	else if (pid==0) {
		// Set state to running.
		state=ProcessState::Running;
		
		// Setup syscall functor.
		info.syscall=syscall;
		info.syscallData=syscallData;
		
		// Child process calls _start - the entry point of the new program.
		(*start)((void *)&info);

		// End this process if we forked.
		if (doFork)
			exit(EXIT_SUCCESS);
	}

	return true;
}

Process *Process::forkCopy(void (*syscall)(void *, uint32_t, ...), void *syscallData) {
	size_t nameSize, pathSize;

	// Create 'blank' child.
	Process *child=new Process;
	if (child==NULL)
		return NULL;
	child->info.argc=0;
	child->info.argv=NULL;
	child->info.syscallData=NULL;
	child->dlHandle=NULL;
	child->name=NULL;
	child->path=NULL;
	
	// argc and argv.
	child->info.argv=(const char **)malloc(sizeof(char *)*info.argc);
	if (child->info.argv==NULL)
		goto error;
	for(child->info.argc=0;child->info.argc<info.argc;++child->info.argc)
	{
		size_t argSize=strlen(info.argv[child->info.argc])+1;
		child->info.argv[child->info.argc]=(const char *)malloc(sizeof(char)*argSize);
		if (child->info.argv[child->info.argc]==NULL)
			goto error;
		memcpy((void *)child->info.argv[child->info.argc], (void *)info.argv[child->info.argc], argSize);
	}
	
	// Name.
	nameSize=strlen(name)+1;
	child->name=(char *)malloc(nameSize);
	if (child->name==NULL)
		goto error;
	memcpy(child->name, name, nameSize);
	
	// Path.
	pathSize=strlen(path)+1;
	child->path=(char *)malloc(pathSize);
	if (child->path==NULL)
		goto error;
	memcpy(child->path, path, pathSize);
	
	// (re)load shared library.
	child->dlHandle=dlopen(path, RTLD_LAZY);
	if (child->dlHandle==NULL)
		goto error;
	
	// Grab _start and main function pointers.
	child->info.main=(ProcessMain)dlsym(child->dlHandle, "main");
	child->start=(ProcessStart)dlsym(child->dlHandle, "_start");
	if (child->info.main==NULL || child->start==NULL)
		goto error;
	
	// Simple stuff.
	info.syscall=syscall;
	info.syscallData=syscallData;
	child->state=state;
	
	return child;
	
	error:
	if (child->info.argv!=NULL) {
		unsigned int i;
		for(i=0;i<child->info.argc;++i)
			free((void *)child->info.argv[i]);
		free((void *)child->info.argv);
	}
	dlclose(child->dlHandle);
	if (child->name!=NULL)
		free((void *)child->name);
	if (child->path!=NULL)
		free((void *)child->path);
	delete child;
	return NULL;
}

ProcessState Process::getState(void)
{
	return state;
}
