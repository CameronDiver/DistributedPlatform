#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "log.h"
#include "process.h"
#include "util.h"

int wrapperKill(pid_t pid, int sig) {
	return kill(pid, sig);
}

Process::Process(void) {
	argc=0;
	argv=NULL;
	environ=NULL;
	name=NULL;
	lPath=NULL;
	state=ProcessState::None;
	posixPid=-1;
	environ=NULL;
	cwd=NULL;
}

Process::~Process(void) {
	// argc and argv.
	if (argv!=NULL) {
		unsigned int i;
		for(i=0;i<argc;++i)
		free((void *)argv[i]);
		free(argv);
		argv=NULL;
	}
	argc=0;

	// Name.
	if (name!=NULL) {
		free(name);
		name=NULL;
	}
	
	// Path.
	if (lPath!=NULL) {
		free(lPath);
		lPath=NULL;
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
	state=ProcessState::None;
	posixPid=-1;
}

bool Process::loadFileFS(Fs *fs, const char *fsPath) {
	// Check we are not already loaded or running.
	if (state!=ProcessState::None)
		return false;

	// Check path is absolute.
	if (fsPath==NULL || fsPath[0]!='/')
		return false;

	char *localPath, *trueCwd;

	// dlopen requires a local file so ask the file system for such a file.
	localPath=fs->fileLocalPath(fsPath);
	if (localPath==NULL)
		goto error;

	// Set current working directory.
	assert(cwd==NULL);
	cwd=(char *)malloc(strlen(fsPath)+1);
	if (cwd==NULL)
		goto error;
	strcpy(cwd, fsPath);
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

	// Special case - env==NULL.
	if (env==NULL) {
		environ=NULL;
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

	return true;

	error:
	if (environ!=NULL) {
		for(ptr2=environ;*ptr2!=NULL;++ptr2)
			free(*ptr2);
		free(environ);
		environ=NULL;
	}
	return false;
}

bool Process::run(unsigned int gargc, ...) {
	va_list ap;
	va_start(ap, gargc);
	bool ret=vrun(gargc, ap);
	va_end(ap);
	return ret;
}

bool Process::vrun(unsigned int gargc, va_list ap) {
	// Setup argc and argv.
	gargc+=2; // for program name and NULL terminator.
	char **gargv=(char **)malloc(sizeof(char *)*gargc);
	if (gargv==NULL)
		return false;
	unsigned int i;
	size_t size=strlen(name)+1;
	gargv[0]=(char *)malloc(sizeof(char)*size);
	if (gargv[0]==NULL) {
		free((void *)gargv);
		return false;
	}
	memcpy((void *)gargv[0], (void *)name, size);
	for(i=1;i<gargc-1;++i) {
		const char *arg=(const char *)va_arg(ap, const char *);
		size_t size=strlen(arg)+1;
		gargv[i]=(char *)malloc(sizeof(char)*size);
		if (gargv[i]==NULL) {
			unsigned int j;
			for(j=0;j<i;++j)
				free((void *)gargv[j]);
			free((void *)gargv);
			return false;
		}
		memcpy((void *)gargv[i], (void *)arg, size);
	}
	gargv[gargc-1]=NULL;

	// Call arun() to do rest of the work.
	if(this->arun((const char **)gargv))
		return true;

	for(i=0;i<gargc;++i)
		free(gargv[i]);
	free(gargv);
	return false;
}

bool Process::arun(const char **gargv) {
	// Ensure program is loaded but not running.
	if (state!=ProcessState::Loaded)
		return false;

	// Fork to create new process, if desired.
	pid_t childPid=fork();
	if (childPid<0)
		return false;
	else if (childPid==0) {
		// Child.

		// Setup ptrace to intercept system calls etc.
		if (ptrace(PTRACE_TRACEME, 0, NULL, NULL))
			exit(EXIT_FAILURE);

		// Exec new process.
		execvpe(lPath, (char *const *)gargv, environ);

		// Exec returned - error, end this process.
		exit(EXIT_FAILURE);
	} else {
		// Parent.

		// Set state to running and update posixPID.
		state=ProcessState::Running;
		setPosixPid(childPid);
		
		// Setup argc and argv.
		argc=0;
		argv=gargv;
		const char **ptr;
		for(ptr=gargv;*ptr!=NULL;++ptr)
			argv[argc++]=*ptr;

		// Wait for exec call to succeed (or error).
		bool success=false, call=false;
		while(!call) {
			int status;
			waitpid(childPid, &status, 0);
			if(WIFEXITED(status))
				break;
			else if(WIFSIGNALED(status)) {
				// Signal - simply allow it to continue.
				// TODO: Do we need to handle any signals?
			} else if (WIFSTOPPED(status)) {
				call=true;

				long long rax=ptrace(PTRACE_PEEKUSER, childPid, 8*ORIG_RAX, NULL);
				if (rax==SYS_execve)
					success=true;
				else
					// Kill process.
					this->kill();
			}

			// Continue program.
			ptrace(PTRACE_SYSCALL, childPid, NULL, NULL);
		}

		if (!success) {
			state=ProcessState::Loaded;
			setPosixPid(-1);
			argv=NULL;
			argc=0;
		}

		return success;
	}
}

Process *Process::forkCopy(void) {
	size_t nameSize, lPathSize, cwdSize;

	// Create 'blank' child.
	Process *child=new Process();
	if (child==NULL)
		return NULL;

	// argc and argv.
	child->argv=(const char **)malloc(sizeof(char *)*argc);
	if (child->argv==NULL)
		goto error;
	for(child->argc=0;child->argc<argc;++child->argc)
	{
		size_t argSize=strlen(argv[child->argc])+1;
		child->argv[child->argc]=(const char *)malloc(sizeof(char)*argSize);
		if (child->argv[child->argc]==NULL)
			goto error;
		memcpy((void *)child->argv[child->argc], (void *)argv[child->argc], argSize);
	}

	// Name.
	nameSize=strlen(name)+1;
	child->name=(char *)malloc(nameSize);
	if (child->name==NULL)
		goto error;
	memcpy(child->name, name, nameSize);
	
	// Path.
	lPathSize=strlen(lPath)+1;
	child->lPath=(char *)malloc(lPathSize);
	if (child->lPath==NULL)
		goto error;
	memcpy(child->lPath, lPath, lPathSize);

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
	if (child->argv!=NULL) {
		unsigned int i;
		for(i=0;i<child->argc;++i)
			free((void *)child->argv[i]);
		free((void *)child->argv);
	}
	free((void *)child->name);
	free((void *)child->lPath);
	free((void *)child->cwd);
	delete child;
	return NULL;
}

ProcessState Process::getState(void) {
	return state;
}

pid_t Process::getPosixPid(void) {
	return posixPid;
}

void Process::setPosixPid(pid_t pid) {
	posixPid=pid;
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

bool Process::isActivity(void) {
	// Check we are running.
	if (state!=ProcessState::Running)
		return false;

	// Use waitid (non-blocking) to check for activity.
	pid_t pid=this->getPosixPid();
	siginfo_t info;
	info.si_pid=0;
	if (waitid(P_PID, (id_t)pid, &info, WEXITED|WSTOPPED|WNOHANG)!=0) {
		// Error.
		this->kill();
		return false;
	}

	if (info.si_pid!=pid)
		return false; // waitid() with WNOHANG still returns success if child isn't waitable.

	return true;
}

bool Process::loadFileLocal(const char *gpath) {
	const char *pathLast;
	size_t nameSize, lPathSize;

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
	lPathSize=strlen(gpath)+1;
	lPath=(char *)malloc(sizeof(char)*lPathSize);
	if (lPath==NULL)
		goto error;
	memcpy((void *)lPath, (void *)gpath, lPathSize);

	// Update state.
	state=ProcessState::Loaded;

	// Success.
	return true;

	error:
	if (name!=NULL) {
		free(name);
		name=NULL;
	}
	if (lPath!=NULL) {
		free(lPath);
		lPath=NULL;
	}
	return false;
}

bool Process::kill(void) {
	if (state!=ProcessState::Running)
		return true;

	state=ProcessState::Killing;
	return (wrapperKill(this->getPosixPid(), SIGKILL)==0);
}