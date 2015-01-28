#ifndef JOB_H
#define JOB_H

#include <vector>

class Job {

 public:
 	Job(char *name);
 	~Job();

 	void addArg(char *arg);

 private:
 	// either the exe or the name of the inbuilt command
 	char *jobName;

 	vector<char *> args;

 	// TODO: inputs, redirects, background process? etc
};


#endif