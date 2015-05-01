#ifndef PROCESS_H
#define PROCESS_H

#include <cstdarg>
#include <cstdint>
#include <vector>
#include <unistd.h>

#include "fs.h"

typedef int32_t ProcessPid;
const ProcessPid ProcessPidError=-1;

enum class ProcessState { None, Loaded, Running };

class Process {
 public:
	std::vector<int> fds; // File descriptors, local-fd index, global-fd value.

	Process(void);
	~Process(void);

	bool loadFileFS(Fs *fs, const char *fsPath);

	const char *getCwd(void);
	bool setCwd(const char *gcwd);

	const char **getEnviron(void);
	bool setEnviron(const char **env);

	bool run(unsigned int gargc=0, ...);
	bool vrun(unsigned int gargc, va_list ap);
	bool arun(const char **gargv); // NULL terminated list of strings.

	Process *forkCopy(void);

	ProcessState getState(void);
	pid_t getPosixPid(void);
	void setPosixPid(pid_t pid);

	int fdAdd(int serverFd); // If successful, adds entry and returns process fd. Otherwise returns -1.
	void fdRemove(int serverFd);
 private:
	char *name;
	char *lPath; // Local path.
	ProcessState state;
	pid_t posixPid;
	char **environ;
	char *cwd; // Current working directory.
	int argc;
	const char **argv;

	bool loadFileLocal(const char *path); // Load from a file in the same layer the server is running.
};

#endif 
