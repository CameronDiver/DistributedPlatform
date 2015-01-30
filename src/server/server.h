#ifndef SERVER_H
#define SERVER_H

#include <cstdarg>
#include <cstddef>
#include <vector>

#include "fs.h"
#include "process.h"

class Server {
 public:
	Server(size_t maxRam=128*1024*1024, size_t maxCores=1);
	~Server(void);

	bool run(FS *fs, const char *initPath);
	
	ProcessPID processFork(ProcessPID parentPID);
 private:
	std::vector<Process> procs;
	
	ProcessPID processAdd(Process *proc);
	bool processRun(ProcessPID pid, bool doFork=true, unsigned int argc=0, ...);
};

#endif 
