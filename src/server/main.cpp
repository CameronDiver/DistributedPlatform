#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fsdirect.h"
#include "log.h"
#include "net/sockettcp.h"
#include "server.h"

void start(void);
void startDaemon(void);

int main(int argc, char **argv) {
	// No arguments?
	if (argc<=1)
		printf("Usage %s start/stop\n", argv[0]);
	else {
		// Check if running by attempting to connect.
		SocketTcp sock;
		bool running=false;
		if (sock.connect("localhost", 51717))
			running=true;

		// Told to stop already running server?
		if (!strcmp(argv[1], "stop") || !strcmp(argv[1], "restart")) {
			// Not running?
			if (!running) {
				printf("Error: Could not find existing instance to stop.\n");
				exit(EXIT_FAILURE);
			}

			// Connect and send command to stop.
			sock.write("stop\n", 5); // TODO: Check written correctly.
			sock.close();

			// TODO: Wait for it to actually exit (in case we are restarting).

			running=false;
		}

		// Told to start new instance?
		if (!strcmp(argv[1], "start") || !strcmp(argv[1], "restart")) {
			// Check not an existing instance.
			if (running) {
				printf("Error: Instance already running.\n");
				exit(EXIT_FAILURE);
			}

			// Start instance.
			start();
		}
	}

	return EXIT_SUCCESS;
}

void start(void) {
	log(LogLevelInfo, "----------------------------------------------------\n");
	log(LogLevelInfo, "Starting up.\n");

	// Become a daemon (fork etc.).
	startDaemon();
	log(LogLevelInfo, "Became a daemon.\n");

	// Load file system which server will 'boot' from.
	const char *containerPath="./container";
	FSDirect fs;
	if (!fs.mountFile(containerPath)) {
		log(LogLevelCrit, "Could not load container '%s'.\n", containerPath);
		exit(EXIT_FAILURE);
	}
	log(LogLevelInfo, "Loaded container filesystem.\n");

	// Create server instance and run init program.
	Server server(51717);
	if (!server.run(&fs, "/sys/init")) {
		log(LogLevelCrit, "Could not run server.\n");
		exit(EXIT_FAILURE);
	}

	log(LogLevelInfo, "Server excited.\n");
}

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