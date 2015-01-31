#include <cstdio>
#include <cstdlib>

#include "command.h"
#include "common.h"
#include "token.h"

#ifndef DSHELL_VERSION
	#define DSHELL_VERSION 0.1
#endif

int main(int argc, char *argv[]) {
	//setup the environment
	Dprintf("dshell v%.2f\n", DSHELL_VERSION);


	// wait for input
	
	// tell getline to allocate it's own buffer 
	size_t size = 0;
	char *input = NULL;
	int count;
	while((count = Dgetline(&input, &size, stdin)) != -1) {
		// process the input
		
		//split the strings up by spaces. 
		// use a custom parser as strings 
		// and other things can contain spaces
		std::vector<Token> tokens;
		tokenize(input, tokens);

		std::vector<char *> jobs;
		handleCommand(tokens, jobs);

		for(int i = 0; i < jobs.size(); ++i) {
			//Dprintf("job: %s\n", jobs[i]);
			Job *proc = Job::jobFromString(jobs[i]);
			proc->startProcess();

			size_t size = 0;
			char *output =NULL;
			int count;
			while((count = Dgetline(&input, &size, proc->getOutputStream())) != -1) {
				Dprintf("%s\n", output);
			}
		}


		// free the tokens
		for(int i = 0; i < tokens.size(); ++i) {
			Token t = tokens[i];
			Dfree(t.str);
		}
		// TODO: free the jobs strings
	}
	Dfree(input);

	return 0;
}
