#include <stdio.h>
#include <stdlib.h>

#ifndef DSHELL_VERSION
	#define DSHELL_VERSION 0.1
#endif

#define Dprintf printf

int main(int argc, char *argv[]) {
	//setup the environment
	Dprintf("dshell v%.2f\n", DSHELL_VERSION);


	// wait for input
	
	// tell getline to allocate it's own buffer 
	size_t size = 0;
	char *input = NULL;
	int count;
	while((count = getline(&input, &size, stdin)) != -1) {
		// process the input
		Dprintf("%i: %s\t [%p]\n", count, input, input);
		
		//split the strings up by spaces. 
		// use a custom parser as strings 
		// and other things can contain spaces
		// tokenize -> work out what job is -> do job

		

	}
	free(input);

	return 0;
}