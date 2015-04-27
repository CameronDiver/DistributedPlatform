#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <libgen.h>

#include "process.h"
#include "util.h"

Process::Process(void) {
	info.argc=0;
	info.argv=NULL;
	info.environ=NULL;
	name=NULL;
	path=NULL;
	state=ProcessState::None;
	posixPID=-1;
	environ=NULL;
	cwd=NULL;
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

	// Current working directory.
	free(cwd);
	cwd=NULL;

	// Others.
	info.environ=NULL; // No need to free as original allocations free'd above.
	state=ProcessState::None;
	posixPID=-1;
}

bool Process::loadFileFS(FS *fs, const char *path) {
	// Check we are not already loaded or running.
	if (state!=ProcessState::None)
		return false;

	// Check path is absolute.
	if (path==NULL || path[0]!='/')
		return false;

	char *localPath, *trueCwd;

	// dlopen requires a local file so ask the file system for such a file.
	localPath=fs->fileLocalPath(path);
	if (localPath==NULL)
		goto error;

	// Set current working directory.
	assert(cwd==NULL);
	cwd=(char *)malloc(strlen(path)+1);
	if (cwd==NULL)
		goto error;
	strcpy(cwd, path);
	trueCwd=dirname(cwd);
	memmove(cwd, trueCwd, strlen(trueCwd)+1);

	// Attempt to load file.
	if (!this->loadFileLocal(localPath))
		goto error;

	// Tidy up.
	free(localPath);
	return true;

	error:
	free(localPath);
	free(cwd);
	cwd=NULL;
	return false;
}

const char *Process::getCwd(void) {
	return cwd;
}

bool Process::setCwd(const char *gcwd) {
	// Allocate memory.
	size_t gSize=strlen(gcwd)+1;
	char *ptr=(char *)malloc(gSize);
	if (ptr==NULL)
		return false;

	// Overwrite old.
	free(cwd);
	cwd=ptr;
	memcpy(cwd, gcwd, gSize);

	return true;
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

	// Special case - env==NULL.
	if (env==NULL) {
		environ=NULL;
		info.environ=NULL;
		return true;
	}

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

bool Process::run(bool doFork, unsigned int argc, ...) {
	va_list ap;
	va_start(ap, argc);
	bool ret=vrun(doFork, argc, ap);
	va_end(ap);
	return ret;
}

bool Process::vrun(bool doFork, unsigned int argc, va_list ap) {
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
	if(this->arun(doFork, argc, (const char **)argv))
		return true;
	
	for(i=0;i<argc;++i)
		free(argv[i]);
	free(argv);
	return false;
}

bool Process::arun(bool doFork, unsigned int argc, const char **argv) {
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
		
		// Setup ptrace to intercept system calls etc.
		// TODO: this (*ptrace*)
		
		// Exec new process.
		// TODO: this. think properly about obeying doFork (*ptrace*)

		// Exec returned - error.
		// TODO: this (*ptrace*)

		// End this process if we forked.
		if (doFork)
			exit(EXIT_SUCCESS);
	}

	return true;
}

Process *Process::forkCopy(void) {
	size_t nameSize, pathSize, cwdSize;

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

	// Current working directory.
	cwdSize=strlen(cwd)+1;
	child->cwd=(char *)malloc(cwdSize);
	if (child->cwd==NULL)
		goto error;
	memcpy(child->cwd, cwd, cwdSize);
	
	// Simple stuff.
	child->state=state;

	// Set environment.
	child->setEnviron(this->getEnviron());

	// TODO: Also copy file descriptors.
	
	return child;
	
	error:
	if (child->info.argv!=NULL) {
		unsigned int i;
		for(i=0;i<child->info.argc;++i)
			free((void *)child->info.argv[i]);
		free((void *)child->info.argv);
	}
	free((void *)child->name);
	free((void *)child->path);
	free((void *)child->cwd);
	delete child;
	return NULL;
}

ProcessState Process::getState(void) {
	return state;
}

pid_t Process::getPosixPID(void) {
	return this->posixPID;
}

void Process::setPosixPID(pid_t pid) {
 	posixPID=pid;
}

int Process::fdAdd(int serverFd) {
	// Look for unused slot.
	size_t i;
	for(i=0;i<fds.size();++i)
		if (fds[i]==-1) {
			fds[i]=serverFd;
			return i;
		}

	// Otherwise add new.
	fds.push_back(serverFd);
	return i;
}

void Process::fdRemove(int serverFd) {
	// TODO: Is serverFd definitely unique among fds?
	size_t i;
	for(i=0;i<fds.size();++i)
		if (fds[i]==serverFd) {
			fds[i]=-1;
			return;
		}
}

bool Process::loadFileLocal(const char *gpath) {
	const char *pathLast;
	size_t nameSize, pathSize;

	// Check we are not already loaded or running.
	if (state!=ProcessState::None)
		return false;

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