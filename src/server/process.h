#ifndef PROCESS_H
#define PROCESS_H

#include <cstdarg>
#include <cstdint>
#include <vector>
#include <unistd.h>

#include "fs.h"

typedef int32_t ProcessPID;
const ProcessPID ProcessPIDError=-1;

enum class ProcessState { None, Loaded, Running };

class Process {
 public:
	typedef struct
	{
		//TODO: Do we need a version number of sorts (to ensure stdlib version of this struct matches)?
		int32_t argc;
		const char **argv; // TODO: Static assert that sizeof(char)==1?
		char **environ;
	} Info;
	std::vector<int> fds; // File descriptors, local-fd index, global-fd value.

	Process(void);
	~Process(void);

	bool loadFileFS(FS *fs, const char *path);

	const char *getCwd(void);
	bool setCwd(const char *gcwd);

	const char **getEnviron(void);
	bool setEnviron(const char **env);

	bool run(bool doFork, unsigned int argc=0, ...);
	bool vrun(bool doFork, unsigned int argc, va_list ap);
	bool arun(bool doFork, unsigned int argc, const char **argv);

	Process *forkCopy(void);

	ProcessState getState(void);
	pid_t getPosixPID(void);
	void setPosixPID(pid_t pid);

	int fdAdd(int serverFd); // If successful, adds entry and returns process fd. Otherwise returns -1.
	void fdRemove(int serverFd);
 private:
	Info info;
	char *name;
	char *path;
	ProcessState state;
	pid_t posixPID;
	char **environ;
	char *cwd; // Current working directory.

	bool loadFileLocal(const char *path); // Load from a file in the same layer the server is running.
};

#endif 
