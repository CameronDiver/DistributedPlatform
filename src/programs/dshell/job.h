#ifndef JOB_H
#define JOB_H

#include <cstdlib>
#include <cstdio>
#include <vector>

#include "common.h"

class Job {

 public:
 	Job(char *name);
 	~Job();

 	void addArg(char *arg);

 	// start the process and setup the standard output
 	// redirection
 	void startProcess();

 	static Job *jobFromString(char *str);

 	FILE *getOutputStream();
 private:
 	// either the exe or the name of the inbuilt command
 	char *jobName;

 	std::vector<char *> args;

 	FILE *stdoutStream;

 	// TODO: inputs, redirects, background process? etc
};


#endif
