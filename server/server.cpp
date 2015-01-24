#include <cassert>
#include <cstdio>

#include "server.h"

Server::Server(size_t maxRam, size_t maxCores) {
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
	if (initPID==-1)
		return false;
	
	// Run (without forking, so blocking).
	if (!this->processRun(initPID, false))
		return false;

	return true;
}

ProcessPID Server::processAdd(Process *proc)
{
	// Check process is loaded but not running (hence ready to run).
	if (proc->getState()!=ProcessState::Loaded)
		return -1;
	
	// Add to queue.
	ProcessPID pid=procs.size();
	procs.push_back(*proc);
	
	return pid;
}

bool Server::processRun(ProcessPID pid, bool doFork)
{
	assert(pid>=0 && pid<procs.size());
	return procs[pid].run(doFork);
}
