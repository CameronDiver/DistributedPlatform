#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fsdirect.h"
#include "server.h"

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
	// TODO: Implement a working signal handler.
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
	//chdir("/");
	// We don't want this overwriting things during the
	// testing phase - c
	chdir("/tmp");

	// Close all open file descriptors.
	int x;
	for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
		close(x);
}

int main() {
	// Become a daemon (fork etc.).
	// Currently disabled to allow easy viewing of output.
	// startDaemon();

	// Load file system which server will 'boot' from.
	const char *containerPath="./container";
	FSDirect fs;
	if (!fs.mountFile(containerPath)) {
		std::cout << "Error: Could not load container '" << containerPath << "'." << std::endl;
		exit(EXIT_FAILURE);
	}

	// Create server instance and run init program.
	Server server(128*1024*1024, 1);
	if (!server.run(&fs, "init/init")) {
		std::cout << "Error: Could not run server." << std::endl;
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
