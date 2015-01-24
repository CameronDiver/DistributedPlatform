#include "../../stdlib/stdio.h"
#include "../../stdlib/stdlib.h"
#include "../../stdlib/unistd.h"

int main(unsigned int argc, char **argv) {
	// Check we are the init process.
	pid_t pid=Dgetpid();
	if (pid!=1)
		return EXIT_FAILURE;
	
	Dprintf("Hello world from init!\n");
	return EXIT_SUCCESS;
}
