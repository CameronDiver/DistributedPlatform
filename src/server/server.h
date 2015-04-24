#ifndef SERVER_H
#define SERVER_H

#include <cstdarg>
#include <cstddef>
#include <list>
#include <netinet/in.h>
#include <sqlite3.h>
#include <vector>

#include "connection.h"
#include "fs.h"
#include "process.h"

class Server {
 public:
	Server(int port=-1);
	~Server(void);

	bool run(FS *fs, const char *initPath);
	void stop(void);

	ProcessPID processFork(ProcessPID parentPID);
	
	std::vector<Process *> procs;
 	FS *filesystem;

 private:
 	const char *pathDatabase="sys/database.db";

	std::list <Connection> connections;
	int tcpSockFd, tcpPort;
	fd_set fdSetActive;
	sqlite3 *database;
	bool stopFlag;

	bool databaseLoad(void);
	ProcessPID processAdd(Process *proc);
	bool processRun(ProcessPID pid, bool doFork=true, unsigned int argc=0, ...);
	bool tcpListen(int port); // Begin listening for other devices over TCP.
	void tcpClose(void);
	void tcpPoll(void);
	bool tcpRead(Connection *con);
};

#endif 
