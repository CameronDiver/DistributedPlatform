#include <stdio.h>
#include <stdlib.h>
#include <sys.h>

int main(unsigned int argc, char **argv) {
	// Check we are the init process.
	pid_t pid=getpid();
	if (pid!=1)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
