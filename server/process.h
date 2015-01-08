#ifndef PROCESS_H
#define PROCESS_H

#include <cstdarg>
#include <cstdint>

typedef int (*ProcessMain)(unsigned int, const char **);
typedef void (*ProcessStart)(const void *);

class Process {
 public:
 	Process(void);
 	~Process(void);
	bool loadFileLocal(const char *path); // Load from a file in the same layer the server is running.
 	bool run(unsigned int argc, ...);
 private:
 	struct
 	{
 		//TODO: Do we need a version number of sorts (to ensure stdlib version of this struct matches)?
		uint16_t argc;
		const char **argv; // TODO: Static assert that sizeof(char)==1?
		ProcessMain main;
 	}info;
 	void *dlHandle;
	ProcessStart start;
	char *name;
};

#endif 
