#include <cstdio>

#include "process.h"
#include "server.h"

Server::Server(size_t maxRam, size_t maxCores) {
}

Server::~Server(void) {
}

bool Server::run(FS *fs, const char *initPath) {
	// Load and run init process.
	Process initProc;
	if (!initProc.loadFileFS(fs, initPath))
		return false;
	if (!initProc.run(false))
		return false;

	return true;
}
