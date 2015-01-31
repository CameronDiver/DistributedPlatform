#include "job.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

Job::Job(char *name) {
	jobName = (char *) Dmalloc(strlen(name) + 1);
	// TODO: check malloc
	Dstrcpy(jobName, name);
}

Job::~Job() {

	// go through all the arguments and free them
	for(int i = 0; i < args.size(); ++i){
		Dfree(args[i]);
	}

	// free the memory reserved for the name
	Dfree(jobName);
}


void Job::addArg(char *arg) {
	char *a = (char *)Dmalloc(strlen(arg) + 1);
	Dstrcpy(a, arg);
	args.push_back(a);
}


void Job::startProcess() {
	// open the process, setup the read stream, and close the stream
	// when the process returns
	stdoutStream = popen(jobName, "we");
}

FILE *Job::getOutputStream() {
	return stdoutStream;
}

Job *Job::jobFromString(char *str) {
	char *exeName = Dstrtok(str, " \n");

	Job *job = new Job(exeName);

	char *arg;
	while((arg = Dstrtok(NULL, " \n")) != NULL) {
		job->addArg(arg);
	}

	return job;
}
