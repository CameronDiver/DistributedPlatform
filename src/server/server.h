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
	FS *filesystem;

	Server(int port=-1);
	~Server(void);

	bool run(FS *fs, const char *initPath);
	void stop(void);

	ProcessPID processFork(ProcessPID parentPID);

	void syscall(ProcessPID pid, int id, va_list ap);

private:
 	const char *pathDatabase="sys/database.db";

	std::list <Connection> connections;
	int tcpSockFd, tcpPort;
	fd_set fdSetActive;
	sqlite3 *database;
	bool stopFlag;
	Devices devices;

	bool databaseLoad(void);
	ProcessPID processAdd(Process *proc);
	void processFree(Process *proc);
	bool processRun(ProcessPID pid, bool doFork=true, unsigned int argc=0, ...);
	bool tcpListen(int port); // Begin listening for other devices over TCP.
	void tcpClose(void);
	void tcpPoll(void);
	bool tcpRead(Connection *con);
	ssize_t fdWrite(int fd, const void *buf, size_t count);
	ssize_t fdRead(int fd, void *buf, size_t count);
	int fdOpenFdFromPath(Process *proc, const char *path, int flags, mode_t mode); // Wrapper around FromFile() and FromDevice() variants.
	int fdOpenFdFromFile(Process *proc, const char *path, int flags, mode_t mode);
	int fdOpenFdFromDevice(Process *proc, const char *name, int flags, mode_t mode);
	int fdOpenSocket(Process *proc, Socket *socket);
	int fdClose(Process *proc, int fd);
	Server::FdEntry *fdGet(int fd);
	int fdCreate(void); // Returns new fd 'slot' to use.
	void syscallExit(ProcessPID pid, uint32_t status);
};

#endif 
