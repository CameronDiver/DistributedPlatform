#include <dlfcn.h>
#include <cstdlib>
#include <cstring>

#include "process.h"
#include "util.h"

Process::Process(void) {
	info.argc=0;
	info.argv=NULL;
	info.main=NULL;
	info.syscall=NULL;
	info.syscallData=NULL;
	info.environ=NULL;
	dlHandle=NULL;
	name=NULL;
	path=NULL;
	state=ProcessState::None;
	posixPID=-1;
	environ=NULL;
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
		free(path);
		path=NULL;
	}
	
	// Environment.
	if (environ!=NULL) {
		char **ptr;
		for(ptr=environ;*ptr!=NULL;++ptr)
			free(*ptr);
		free(environ);
		environ=NULL;
	}

	// Others.
	info.syscall=NULL;
	info.syscallData=NULL;
	info.environ=NULL; // No need to free as original allocations free'd above.
	state=ProcessState::None;
	posixPID=-1;
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

	// Grab function pointers.
	info.main=(ProcessMain)dlsym(dlHandle, "main");
	start=(ProcessStart)dlsym(dlHandle, "_start");
	restart=(ProcessStart)dlsym(dlHandle, "_restart");
	if (info.main==NULL || start==NULL || restart==NULL)
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

const char **Process::getEnviron(void) {
	return (const char **)environ;
}

bool Process::setEnviron(const char **env) {
	// Clear old.
	const char **ptr;
	char **ptr2;
	if (environ!=NULL) {
		for(ptr2=environ;*ptr2!=NULL;++ptr2)
			free(*ptr2);
		free(environ);
		environ=NULL;
	}
	info.environ=environ;

	// Copy new.
	for(ptr=env;*ptr!=NULL;++ptr) ;
	environ=(char **)malloc(sizeof(char *)*(ptr+1-env));
	if (environ==NULL)
		goto error;
	for(ptr=env,ptr2=environ;*ptr!=NULL;++ptr,++ptr2) {
		size_t len=strlen(*ptr);
		*ptr2=(char *)malloc(len+1);
		if (*ptr2==NULL)
			goto error;
		strcpy(*ptr2, *ptr);
	}
	*ptr2=NULL;

	info.environ=environ;

	return true;

	error:
	if (environ!=NULL) {
		for(ptr2=environ;*ptr2!=NULL;++ptr2)
			free(*ptr2);
		free(environ);
		environ=NULL;
	}
	info.environ=environ;
	return false;
}

bool Process::run(void (*syscall)(void *, uint32_t, ...), void *syscallData, bool doFork, unsigned int argc, ...) {
	va_list ap;
	va_start(ap, argc);
	bool ret=vrun(syscall, syscallData, doFork, argc, ap);
	va_end(ap);
	return ret;
}

bool Process::vrun(void (*syscall)(void *, uint32_t, ...), void *syscallData, bool doFork, unsigned int argc, va_list ap) {
	// Setup argc and argv.
	++argc; // for program name.
	char **argv=(char **)malloc(sizeof(char *)*argc);
	if (argv==NULL)
		return false;
	unsigned int i;
	size_t size=strlen(name)+1;
	argv[0]=(char *)malloc(sizeof(char)*size);
	if (argv[0]==NULL) {
		free((void *)argv);
		return false;
	}
	memcpy((void *)argv[0], (void *)name, size);
	for(i=1;i<argc;++i) {
		const char *arg=(const char *)va_arg(ap, const char *);
		size_t size=strlen(arg)+1;
		argv[i]=(char *)malloc(sizeof(char)*size);
		if (argv[i]==NULL) {
			unsigned int j;
			for(j=0;j<i;++j)
				free((void *)argv[j]);
			free((void *)argv);
			return false;
		}
		memcpy((void *)argv[i], (void *)arg, size);
	}

	// Call arun() to do rest of the work.
	if(this->arun(syscall, syscallData, doFork, argc, (const char **)argv))
		return true;
	
	for(i=0;i<argc;++i)
		free(argv[i]);
	free(argv);
	return false;
}

bool Process::arun(void (*syscall)(void *, uint32_t, ...), void *syscallData, bool doFork, unsigned int argc, const char **argv) {
	// TODO: Check argc can fit into type.
	
	// Ensure program is loaded but not running.
	if (state!=ProcessState::Loaded)
		return false;

	// Fork to create new process, if desired.
	pid_t pid=(doFork ? fork() : 0);
	if (pid<0)
		return false;
	else if (pid==0) {
		// Set state to running and update posixPID.
		state=ProcessState::Running;
		posixPID=getpid();
		
		// Setup argc and argv.
		info.argc=argc;
		info.argv=argv;
		
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
	Process *child=new Process();
	if (child==NULL)
		return NULL;
	
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
	
	// Grab function pointers.
	child->info.main=(ProcessMain)dlsym(child->dlHandle, "main");
	child->start=(ProcessStart)dlsym(child->dlHandle, "_start");
	child->restart=(ProcessRestart)dlsym(child->dlHandle, "_restart");
	if (child->info.main==NULL || child->start==NULL || child->restart==NULL)
		goto error;
	
	// Simple stuff.
	child->info.syscall=syscall;
	child->info.syscallData=syscallData;
	child->state=state;

	// Set environment.
	child->setEnviron(this->getEnviron());

	// Restart stdlib as we've changed the info struct.
	(*child->restart)(&child->info);
	
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

pid_t Process::getPosixPID(void) {
	return this->posixPID;
}

void Process::setPosixPID(pid_t pid) {
 	posixPID=pid;
}
