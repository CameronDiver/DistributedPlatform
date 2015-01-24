#ifndef PROCESS_H
#define PROCESS_H

#include <cstdarg>
#include <cstdint>

#include "fs.h"

typedef uint32_t ProcessPID;
const ProcessPID ProcessPIDError=0;

enum class ProcessState { None, Loaded, Running };

typedef int (*ProcessMain)(unsigned int, const char **);
typedef void (*ProcessStart)(const void *);

class Process {
 public:
 	Process(void);
 	~Process(void);
 	
	bool loadFileLocal(const char *path); // Load from a file in the same layer the server is running.
	bool loadFileFS(FS *fs, const char *path);
	
 	bool run(void (*syscall)(void *, uint32_t, ...), void *syscallData, bool doFork=true, unsigned int argc=0, ...);
 	
 	ProcessState getState(void);
 private:
 	struct
 	{
 		//TODO: Do we need a version number of sorts (to ensure stdlib version of this struct matches)?
		uint16_t argc;
		const char **argv; // TODO: Static assert that sizeof(char)==1?
		ProcessMain main;
		void (*syscall)(void *, uint32_t, ...);
		void *syscallData;
 	}info;
 	void *dlHandle;
	ProcessStart start;
	char *name;
	ProcessState state;
};

#endif 
