#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "server.h"

extern "C" void serverSysCall(void *gdata, uint32_t id, ...)
{
	// Find pid.
	Server *server=(Server *)gdata;
	pid_t posixPID=getpid();
	ProcessPID pid=ProcessPIDError;
	unsigned int i;
	for(i=0;i<server->procs.size();++i)
		if (server->procs[i].getPosixPID()==posixPID) {
			pid=i;
			break;
		}
	if (pid==ProcessPIDError)
		return;
	Process *curr=&server->procs[pid];
	
	// Parse system call.
	va_list ap;
	va_start(ap, id);
	switch(id)
	{
		case 0: // exit
		{
			uint32_t status=(uint32_t)va_arg(ap, uint32_t);
			exit(status);
		}
		break;
		case 1: // fork
		{
	  	int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
			*ret=server->processFork(pid);
	  }
		break;
		case 2: // getpid
	  {
	  	int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
	  	*ret=pid;
	  }
		break;
		case 3: // alloc
		{
			// TODO: Take server wide maxRam into account.
			void **ret=(void **)va_arg(ap, void **);
			void *ptr=(void *)va_arg(ap, void *);
			size_t size=(size_t)va_arg(ap, size_t);
			*ret=realloc(ptr, size);
		}
		break;
		case 4: // exec
		{
			const char *path=(const char *)va_arg(ap, const char *);
			uint32_t argc=(uint32_t)va_arg(ap, uint32_t);
			char **argv=(char **)va_arg(ap, char **);
			
			// Create and load new process.
			Process *newProc=new Process;
			if (newProc->loadFileFS(server->filesystem, path))
			{
				// 'Unload' old process.
				curr->~Process();
				
				// Update process array.
				server->procs[pid]=*newProc;
				
				// Run (without forking).
				newProc->arun(&serverSysCall, (void *)server, false, argc, (const char **)argv);
				exit(EXIT_SUCCESS);
			}
		}
		break;
		default:
			// TODO: What to do in case of invalid syscall?
		break;
	}
	
	va_end(ap);
}

Server::Server(size_t maxRam, size_t maxCores) {
	Process dummyProc; // To create PID of 1 for init.
	procs.push_back(dummyProc);
	
	filesystem=NULL;
}

Server::~Server(void) {
}

bool Server::run(FS *fs, const char *initPath) {
	// Setup file system.
	filesystem=fs;
	
	// Load init process.
	Process initProc;
	if (!initProc.loadFileFS(fs, initPath))
		return false;
	
	// Add to list of processes.
	ProcessPID initPID=Server::processAdd(&initProc);
	if (initPID==ProcessPIDError)
		return false;
	
	// Run (without forking, so blocking).
	if (!this->processRun(initPID, false))
		return false;

	return true;
}

ProcessPID Server::processFork(ProcessPID parentPID) {
	// Create child process.
	Process *parent=&procs[parentPID];
	Process *child=parent->forkCopy(&serverSysCall, (void *)this);
	if (child==NULL)
		return ProcessPIDError;
	
	// Add child to list of processes.
	ProcessPID childPID=procs.size();
	procs.push_back(*child);
	
	// Fork.
	pid_t pid=fork();
	if (pid<0)
	{
		// Error.
		procs.pop_back();
		return ProcessPIDError;
	}
  else if (pid==0)
  {
  	procs[childPID].setPosixPID(getpid());
  	return 0; // fork() returns 0 to child.
  }
  else
  	return childPID; // fork() return child pid to parent.
}

ProcessPID Server::processAdd(Process *proc) {
	// Check process is loaded but not running (hence ready to run).
	if (proc->getState()!=ProcessState::Loaded)
		return ProcessPIDError;
	
	// Add to queue.
	ProcessPID pid=procs.size();
	procs.push_back(*proc);
	
	return pid;
}

bool Server::processRun(ProcessPID pid, bool doFork, unsigned int argc, ...) {
	assert(pid>=1 && pid<procs.size());
	
	// Run process.
	va_list ap;
	va_start(ap, argc);
	bool ret=procs[pid].vrun(&serverSysCall, (void *)this, doFork, argc, ap);
	va_end(ap);
	
	return ret;
}
