#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "process.h"

void startDaemon(void) {
	// Fork the parent process.
	pid_t pid = fork();
	if (pid < 0)
		exit(EXIT_FAILURE);

	// Success; terminate the parent.
	if (pid > 0)
		exit(EXIT_SUCCESS);

	// On success the child process becomes session leader.
	if (setsid() < 0)
		exit(EXIT_FAILURE);

	// Catch, ignore and handle signals.
	//TODO: Implement a working signal handler.
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	// Fork off for the second time.
	pid = fork();
	if (pid < 0)
		exit(EXIT_FAILURE);

	// Success; terminate the parent.
	if (pid > 0)
		exit(EXIT_SUCCESS);

	// Set new file permissions.
	umask(0);

	// Change the working directory to the root directory.
	// TODO: Change this to a more sensible place?
	chdir("/");

	// Close all open file descriptors.
	int x;
	for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
		close(x);
}

int main() {
	// Become a daemon (fork etc.).
	// Currently disabled to allow easy viewing of output.
	// startDaemon();

	// For now, load and run the args.so program.
	Process proc;
	if (!proc.loadFileLocal("../programs/args/args.so")) {
		printf("Error: Could not load program.\n");
		return EXIT_FAILURE;
	}

	if (!proc.run(2, "hello", "world!")) {
		printf("Error: Could not run program.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
