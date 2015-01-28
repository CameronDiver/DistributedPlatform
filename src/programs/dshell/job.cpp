#include "job.h"

#include <cstdio>
#include <cstdlib>

Job::Job(char *name) {
	jobName = (char *) malloc(strlen(name) + 1);
	// TODO: check malloc
	strcpy(jobName, name);
}

Job::~Job() {

	// go through all the arguments and free them
	for(int i = 0; i < args.length; ++i){
		free(args[i]);
	}

	// free the memory reserved for the name
	free(jobName);
}


void Job::addArg(char *arg) {
	char *a = (char *)malloc(strlen(arg) + 1);
	strcpy(a, arg);
	args.push_back(a);
}