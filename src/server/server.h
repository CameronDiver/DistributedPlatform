#ifndef SERVER_H
#define SERVER_H

#include <cstdarg>
#include <cstddef>
#include <list>
#include <netinet/in.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <vector>

#include "connection.h"
#include "devices.h"
#include "devices/device.h"
#include "fs.h"
#include "process.h"

class Server {
public:
	typedef enum {
		FdTypeNone,
		FdTypeFd,
		FdTypeSocket,
		FdTypeDevice,
	} FdType;

	typedef struct {
		int fd;
		FdType type;
		unsigned int refCount;
		union {
			int fd;
			Socket *socket;
			Device *device;
		} d;
	} FdEntry;

	std::vector<FdEntry *> fdEntries;
	std::vector<Process *> procs;
	Fs *filesystem;

	Server(int port=-1);
	~Server(void);

	bool run(Fs *fs, const char *initPath);
	void stop(void);

	ProcessPid processFork(ProcessPid parentPid);

private:
 	const char *pathDatabase="sys/database.db";

	std::list <Connection> connections;
	int tcpSockFd, tcpPort;
	fd_set fdSetActive;
	sqlite3 *database;
	bool stopFlag;
	Devices devices;

	bool databaseLoad(void);
	ProcessPid processAdd(Process *proc);
	void processFree(Process *proc);
	bool processRun(ProcessPid pid, unsigned int argc=0, ...);
	void processPoll(ProcessPid pid); // Check for system calls, etc.
	unsigned int stopProcesses(void); // Returns number of proccesses who wouldn't die.
	bool tcpListen(int port); // Begin listening for other devices over TCP.
	void tcpClose(void);
	void tcpPoll(void);
	bool tcpRead(Connection *con);
	ssize_t fdWrite(int fd, const void *buf, size_t count); // Expects 'server' fd (not process).
	ssize_t fdRead(int fd, void *buf, size_t count); // Expects 'server' fd (not process).
	int fdOpenFdFromPath(Process *proc, const char *path, int flags, mode_t mode); // Wrapper around FromFile() and FromDevice() variants.
	int fdOpenFdFromFile(Process *proc, const char *path, int flags, mode_t mode);
	int fdOpenFdFromDevice(Process *proc, const char *name, int flags, mode_t mode);
	int fdOpenSocket(Process *proc, Socket *socket);
	bool fdClose(Process *proc, int fd); // Expects 'server' fd (not process).
	Server::FdEntry *fdGet(int fd); // Expects 'server' fd (not process).
	int fdCreate(void); // Returns new fd 'slot' to use.
};

#endif 
