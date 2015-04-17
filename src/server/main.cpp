#include <cstdlib>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fsdirect.h"
#include "log.h"
#include "server.h"

void startDaemon(void) {
	// Fork the parent process.
	pid_t pid = fork();
	if (pid < 0) {
		log(LogLevelCrit, "Error: Could not fork to init daemon (first time).\n");
		exit(EXIT_FAILURE);
	}

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
	if (pid < 0) {
		log(LogLevelCrit, "Error: Could not fork to init daemon (second time).\n");
		exit(EXIT_FAILURE);
	}

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
	//chdir("/tmp");

	// Close all open file descriptors.
	int x;
	for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
		close(x);
}

int main(int argc, char **argv) {
	log(LogLevelInfo, "----------------------------------------------------\n");
	log(LogLevelInfo, "Starting up.\n");

	// Become a daemon (fork etc.).
	startDaemon();
	log(LogLevelInfo, "Became a daemon.\n");

	// Load file system which server will 'boot' from.
	const char *containerPath="./container";
	FSDirect fs;
	if (!fs.mountFile(containerPath)) {
		log(LogLevelCrit, "Error: Could not load container '%s'.\n", containerPath);
		exit(EXIT_FAILURE);
	}
	log(LogLevelInfo, "Loaded container filesystem.\n");

	// Create server instance and run init program.
	Server server(51717);
	if (!server.run(&fs, "sys/init")) {
		log(LogLevelCrit, "Error: Could not run server.\n");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}