#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "server.h"

typedef struct
{
	Server *server;
	ProcessPID pid;
}ServerSysCallData;

extern "C" void serverSysCall(void *gdata, uint32_t id, ...)
{
	ServerSysCallData *data=(ServerSysCallData *)gdata;
	
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
			*ret=data->server->processFork(data->pid);
	  }
		break;
		case 2: // getpid
	  {
	  	int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
	  	*ret=data->pid;
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
		default:
			// TODO: What to do in case of invalid syscall?
		break;
	}
	
	va_end(ap);
}

Server::Server(size_t maxRam, size_t maxCores) {
	Process dummyProc; // To create PID of 1 for init.
	procs.push_back(dummyProc);
}

Server::~Server(void) {
}

bool Server::run(FS *fs, const char *initPath) {
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
	// Child will require own system call data.
	ServerSysCallData *data=(ServerSysCallData *)malloc(sizeof(ServerSysCallData));
	if (data==NULL)
		return false;
	data->server=this;
	
	// Create child process.
	Process *parent=&procs[parentPID];
	Process *child=parent->forkCopy(&serverSysCall, (void *)data);
	if (child==NULL)
	{
		free(data);
		return ProcessPIDError;
	}
	
	// Add child to list of processes.
	ProcessPID childPID=procs.size();
	data->pid=childPID;
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
  	return 0; // fork() returns 0 to child.
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
	
	// Allocate syscall data structure.
	ServerSysCallData *data=(ServerSysCallData *)malloc(sizeof(ServerSysCallData));
	if (data==NULL)
		return false;
	data->server=this;
	data->pid=pid;
	
	// Run process.
	va_list ap;
	va_start(ap, argc);
	bool ret=procs[pid].vrun(&serverSysCall, (void *)data, doFork, argc, ap);
	va_end(ap);
	
	// If process could not be created, no longer need system call data.
	if (ret==false)
		free(data);
	
	return ret;
}
