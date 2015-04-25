#ifndef PROCESS_H
#define PROCESS_H

#include <cstdarg>
#include <cstdint>
#include <list>
#include <unistd.h>

#include "fs.h"
#include "../misc/syscommon.h"

typedef int32_t ProcessPID;
const ProcessPID ProcessPIDError=-1;

enum class ProcessState { None, Loaded, Running };

typedef void (*ProcessStart)(const void *);
typedef void (*ProcessRestart)(const void *);

class Process {
 public:
	std::list<int> fds; // File descriptors.

 	Process(void);
 	~Process(void);

	bool loadFileFS(FS *fs, const char *path);

 	const char *getCwd(void);
	bool setCwd(const char *gcwd);

	const char **getEnviron(void);
	bool setEnviron(const char **env);

 	bool run(void (*syscall)(void *, uint32_t, ...), void *syscallData, bool doFork, unsigned int argc=0, ...);
 	bool vrun(void (*syscall)(void *, uint32_t, ...), void *syscallData, bool doFork, unsigned int argc, va_list ap);
 	bool arun(void (*syscall)(void *, uint32_t, ...), void *syscallData, bool doFork, unsigned int argc, const char **argv);

 	Process *forkCopy(void (*syscall)(void *, uint32_t, ...), void *syscallData);

	ProcessState getState(void);
	pid_t getPosixPID(void);
	void setPosixPID(pid_t pid);
 private:
	SysCommonProcInfo info;
	void *dlHandle;
	ProcessStart start;
	ProcessRestart restart;
	char *name;
	char *path;
	ProcessState state;
	pid_t posixPID;
	char **environ;
	char *cwd; // Current working directory.

	bool loadFileLocal(const char *path); // Load from a file in the same layer the server is running.
};

#endif 
