#include <stdio.h>
#include <stdlib.h>
#include <sys.h>

int main(unsigned int argc, char **argv) {
	// Check we are the init process.
	pid_t pid=getpid();
	if (pid!=1)
		return EXIT_FAILURE;
	
	Dprintf("Hello world from init!\n");
	return EXIT_SUCCESS;
}
