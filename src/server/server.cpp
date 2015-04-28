#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "devices/devfull.h"
#include "devices/devnull.h"
#include "devices/devsocket.h"
#include "devices/devzero.h"
#include <fcntl.h>
#include <netdb.h>
#include "net/sockettcp.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "server.h"

static int callbackInc(void *user, int argc, char **argv, char **azColName){
   (*((int *)user))++;
   return 0;
}

Server::Server(int port) {
	Process *dummy=new Process();
	procs.push_back(dummy); // To create PID of 1 for init.
	
	tcpPort=port;
	tcpSockFd=-1;
	filesystem=NULL;
	database=NULL;
	stopFlag=false;
}

Server::~Server(void) {
}

bool Server::run(FS *fs, const char *initPath) {
	// Setup file system.
	filesystem=fs;

	// Load database.
	if (!this->databaseLoad())
		log(LogLevelWarning, "Could not load database.\n");
	else
		log(LogLevelInfo, "Loaded database.\n");

	// Add standard devices.
	if (!devices.add("full", new DeviceFull()))
		log(LogLevelErr, "Could not add device 'full'.\n");
	if (!devices.add("null", new DeviceNull()))
		log(LogLevelErr, "Could not add device 'null'.\n");
	if (!devices.add("zero", new DeviceZero()))
		log(LogLevelErr, "Could not add device 'zero'.\n");

	// Load init process.
	Process *initProc=new Process();
	if (!initProc->loadFileFS(fs, initPath)) {
		log(LogLevelCrit, "Could not load init process at '%s'.\n", initPath);
		return false;
	}
	else
		log(LogLevelInfo, "Loaded init process.\n");
	
	// Add to list of processes.
	ProcessPID initPID=this->processAdd(initProc);
	if (initPID==ProcessPIDError) {
		log(LogLevelCrit, "Could not add init process to list.\n");
		return false;
	}
	else
		log(LogLevelInfo, "Added init process.\n");

	// Run init process (forking, so non-blocking).
	if (!this->processRun(initPID, true)) {
		log(LogLevelCrit, "Could not run init process.\n");
		return false;
	}
	else
		log(LogLevelInfo, "Ran init process.\n");

	// Setup TCP socket for listening.
	if (tcpPort>=0) {
		if (!this->tcpListen(tcpPort)) {
			log(LogLevelCrit, "Could not setup TCP socket for listening on port %u\n", tcpPort);
			return false;
		}
		else
			log(LogLevelInfo, "Setup TCP socket for listening on port %u.\n", tcpPort);
	}
	FD_ZERO(&fdSetActive);
	FD_SET(tcpSockFd, &fdSetActive);

	// Main loop.
	while(!this->stopFlag) {
		// Check for any new TCP activity.
		this->tcpPoll();

		// Sleep.
		usleep(10000);
	}

	// Tidy up.
	log(LogLevelInfo, "Stopping.\n");

	// Stop processes.
	// TODO: this (and also tidy up 'procs').

	// Close connections.
	// TODO: this (and also tidy up 'connections').

	// Free file entries.
	// TODO: this

	// Close TCP listening socket.
	this->tcpClose();

	// Close database.
	sqlite3_close(database);
	database=NULL;

	return true;
}

void Server::stop(void) {
	log(LogLevelInfo, "Stop request received.\n");
	this->stopFlag=true;
}

ProcessPID Server::processFork(ProcessPID parentPID) {
	// Create child process.
	Process *parent=procs[parentPID];
	Process *child=parent->forkCopy();
	if (child==NULL)
		return ProcessPIDError;
	
	// Add child to list of processes.
	ProcessPID childPID=procs.size();
	procs.push_back(child);
	
	// Fork.
	pid_t pid=fork();
	if (pid<0)
	{
		// Error.
		procs.pop_back();
		return ProcessPIDError;
	}
	else if (pid==0)
	{
		procs[childPID]->setPosixPID(getpid());

		// Setup ptrace to intercept system calls etc.
		// TODO: this (is it even correct?)

		return 0; // fork() returns 0 to child.
	}
	else
		return childPID; // fork() return child pid to parent.
}

bool Server::databaseLoad(void) {
	// Attempt to open/create database.
	char *localPath=filesystem->fileLocalPath(pathDatabase);
	int ret=sqlite3_open_v2(localPath, &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	free(localPath);
	if (ret) {
		log(LogLevelWarning, "Could not open database at '%s': %s\n", pathDatabase, sqlite3_errmsg(database));
		database=NULL;
		return false;
	}

	// Create DEVICES table if needed.
	char *err=NULL;
	int results=0;
	ret=sqlite3_exec(database, "SELECT name FROM sqlite_master WHERE type='table' AND name='DEVICES';", callbackInc, &results, &err);
	if (results==0) {
		log(LogLevelInfo, "No table 'DEVICES' found - creating.\n");
		ret=sqlite3_exec(database, "CREATE TABLE DEVICES(ID INT PRIMARY KEY NOT NULL, NAME TEXT NOT NULL);", NULL, NULL, &err);

		if (ret!=SQLITE_OK) {
			log(LogLevelWarning, "Could not create table 'DEVICES': %s\n", err);
			sqlite3_free(err);
			sqlite3_close(database);
			database=NULL;
			return false;
		}else
			log(LogLevelInfo, "Created table 'DEVICES'.\n");
	}

	return true;
}

ProcessPID Server::processAdd(Process *proc) {
	// Check process is loaded but not running (hence ready to run).
	if (proc->getState()!=ProcessState::Loaded)
		return ProcessPIDError;
	
	// Add to queue.
	ProcessPID pid=procs.size();
	procs.push_back(proc);
	
	return pid;
}

void Server::processFree(Process *proc) {
	// Close all open file descriptors.
	size_t procFd;
	for(procFd=0;procFd<proc->fds.size();++procFd) {
		int serverFd=proc->fds[procFd];
		if (serverFd==-1)
			continue;

		if (!this->fdClose(proc, serverFd))
			log(LogLevelErr, "Could not close a file descriptor when freeing process.\n");
	}

	// Free.
	delete proc;
}

bool Server::processRun(ProcessPID pid, unsigned int argc, ...) {
	assert(pid>=1 && pid<procs.size());
	
	// Run process.
	va_list ap;
	va_start(ap, argc);
	bool ret=procs[pid]->vrun(argc, ap);
	va_end(ap);
	
	return ret;
}

bool Server::tcpListen(int port) {
	tcpSockFd=socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (tcpSockFd<0)
		return false;

	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));

	servAddr.sin_family=AF_INET;
	servAddr.sin_addr.s_addr=INADDR_ANY;
	servAddr.sin_port=htons(port);
	if (bind(tcpSockFd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0)
		return false;

	return (listen(tcpSockFd,5)==0);
}

void Server::tcpClose(void) {
	if (tcpSockFd!=-1) {
		close(tcpSockFd);
		tcpSockFd=-1;
	}
}

void Server::tcpPoll(void) {
	// Check we are actually listening.
	if (tcpSockFd<0)
		return;

	fd_set fdSetRead=fdSetActive;
	struct timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=0;
	if (select(FD_SETSIZE, &fdSetRead, NULL, NULL, &timeout)<0)
		return;

	// Check for new connect requests.
	if (FD_ISSET(tcpSockFd, &fdSetRead)) {
		struct sockaddr_in addr;
		socklen_t addrLen=sizeof(addr);
		int newFd=accept(tcpSockFd, (struct sockaddr *)&addr, &addrLen);

		if (newFd>=0) {
			// Success, add to list of connections.
			SocketTcp *socket=new SocketTcp();
			socket->listen(&addr, addrLen, newFd);
			connections.push_back(Connection(socket));
			FD_SET(newFd, &fdSetActive);
			log(LogLevelInfo, "New connection from host %s, port %u.\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		}
	}

	// Check for data from TCP clients.
	std::list<Connection>::iterator iter;
	for(iter=connections.begin();iter!=connections.end();) {
		SocketTcp *sockTcp;
		int clientFd;

		// We are only looking for TCP sockets.
		if (iter->sock->type!=Socket::TypeTcp)
			goto next;

		// Check for activity;
		sockTcp=dynamic_cast<SocketTcp *>(iter->sock);
		clientFd=sockTcp->sockFd;
		if (!FD_ISSET(clientFd, &fdSetRead))
			goto next;

		// Read the data we have received.
		if (this->tcpRead(&*iter))
			goto next;

		// Error.
		log(LogLevelInfo, "TCP client disconnected.\n");

		// Close connection.
		close(clientFd);

		// Remove from connections list.
		iter=connections.erase(iter);

		// Clear active flag.
		FD_CLR(clientFd, &fdSetActive);

		// Advance to next connection.
		next:
		++iter;
	}
}

bool Server::tcpRead(Connection *con) {
	SocketTcp *sockTcp=dynamic_cast<SocketTcp *>(con->sock);
	int fd=sockTcp->sockFd;

	// Attempt to read.
	char buffer[1024];
	int nbytes=read(fd, buffer, 1024);
	if (nbytes<=0)
		return false; // EOF or error.

	// Remove line endings.
	do {
		buffer[--nbytes]='\0';
	} while(nbytes>0 && (buffer[nbytes-1]==0 || buffer[nbytes-1]==10 || buffer[nbytes-1]==13));

	// Parse message.
	char *save=NULL;
	char *part=strtok_r(buffer, " ", &save);
	if (part==NULL)
		return true;
	if (!strcmp(part, "type")) {
		// Check if connection already has a type.
		if (con->type!=Connection::TypeNone) {
			// TODO: Should we notify the other party?
			log(LogLevelDebug, "Type command received but connection already has a type.\n"); // TODO: Add 'where from' details.
			return true;
		}

		// Extract desired type.
		part=strtok_r(NULL, " ", &save);
		if (part==NULL) {
			// TODO: Should we notify the other party?

			log(LogLevelDebug, "No type given in type command.\n"); // TODO: Add 'where from' details.
			return true;
		}

		if (!strcmp(part, "tty")) {
			// TODO: Should we notify the other party of the result?

			// Connect socket to a special device file.
			char name[16];
			unsigned int i;
			for(i=0;i<1024;++i) {
				sprintf(name, "tty%u", i);
				if (!devices.exists(name))
					break;
			}
			log(LogLevelDebug, "name: %s\n", name);
			if (!devices.add(name, new DeviceSocket(con->sock)))
				return true;

			// Start new shell.
			Process *shell=new Process();
			if (!shell->loadFileFS(filesystem, "/bin/shell"))
				return true;

			// Add to list of programs.
			ProcessPID shellPID=this->processAdd(shell);
			if (shellPID==ProcessPIDError)
				return true;

			// Setup stdin and stdout.
			// TODO: Connect connection's socket with processes stdin/stdout.

			// Run shell, initially running user's .profile script, then taking input from stdin.
			// TODO: Set tty as stdin/stdout instead of passing as argument.
			char temp[1024];
			sprintf(temp, "/dev/%s", name);
//			if (!this->processRun(shellPID, true, 1, "/home/.profile"))
			if (!this->processRun(shellPID, true, 1, temp))
				return true;

			// Update connection type.
			con->type=Connection::TypeTTY;
			log(LogLevelDebug, "Connection now has type 'tty'.\n"); // TODO: Add 'where from' details.
		} else {
			// TODO: Should we notify the other party?

			log(LogLevelDebug, "Invalid type  '%s'.\n", part); // TODO: Add 'where from' details.
		}
	} else if (!strcmp(part, "stop"))
		this->stop(); // TODO: Check that this connection has permission to stop the server.
	else
		log(LogLevelDebug, "Received bad command '%s'.\n", part); // TODO: Add 'where from' details.

	return true;
}

ssize_t Server::fdWrite(int fd, const void *buf, size_t count) {
	FdEntry *fdEntry=this->fdGet(fd);
	if (fdEntry==NULL)
		return -1;

	switch(fdEntry->type) {
		case FdTypeNone:
			return -1;
		break;
		case FdTypeFd:
			return write(fdEntry->d.fd, buf, count);
		break;
		case FdTypeSocket:
			return fdEntry->d.socket->write(buf, count);
		break;
		case FdTypeDevice:
			return fdEntry->d.device->write(buf, count);
		break;
	}

	return -1;
}

ssize_t Server::fdRead(int fd, void *buf, size_t count) {
	FdEntry *fdEntry=this->fdGet(fd);
	if (fdEntry==NULL)
		return -1;

	switch(fdEntry->type) {
		case FdTypeNone:
			return -1;
		break;
		case FdTypeFd:
			return read(fdEntry->d.fd, buf, count);
		break;
		case FdTypeSocket:
			return fdEntry->d.socket->read(buf, count);
		break;
		case FdTypeDevice:
			return fdEntry->d.device->read(buf, count);
		break;
	}

	return -1;
}

int Server::fdOpenSocket(Process *proc, Socket *socket) {
	// Assumes socket is valid and open.

	// Create FdEntry and add to list.
	int fd=this->fdCreate();
	if (fd==-1)
		return -1;
	FdEntry *entry=this->fdGet(fd);
	entry->type=FdTypeSocket;
	entry->refCount=1;
	entry->d.socket=socket;

	// Add internal fd to proc's list.
	int procFd=proc->fdAdd(entry->fd);
	if (procFd==-1) {
		// TODO: Remove above entry from entries list.
		return -1;
	}

	return procFd;
}

int Server::fdOpenFdFromPath(Process *proc, const char *path, int flags, mode_t mode) {
	// Assumes path has already been 'simplified' and process has valid permissions.

	// Switch on the 'file' type.
	if (!strncmp(path, "/dev/", 5))
		return fdOpenFdFromDevice(proc, path+5, flags, mode);
	else
		return fdOpenFdFromFile(proc, path, flags, mode);

	return -1;
}

int Server::fdOpenFdFromFile(Process *proc, const char *path, int flags, mode_t mode) {
	char *localPath=NULL;
	int pathFd=-1, fd=-1, procFd=-1;
	FdEntry *entry=NULL;

	// Find local path.
	localPath=filesystem->fileLocalPath(path);
	if (localPath==NULL)
		goto error;

	// Call open(2).
	pathFd=open(localPath, flags, mode);
	if (pathFd==-1)
		goto error;

	// Create and add FdEntry to list.
	fd=this->fdCreate();
	if (fd==-1)
		goto error;
	entry=this->fdGet(fd);
	entry->type=FdTypeFd;
	entry->refCount=1;
	entry->d.fd=pathFd;

	// Add internal fd to proc's list.
	procFd=proc->fdAdd(entry->fd);
	if (procFd==-1) {
		// TODO: Remove above entry from entries list.
		return -1;
	}

	free(localPath);
	return procFd;

	error:
	free(localPath);
	if (pathFd>=0)
		close(pathFd);
	return -1;
}

int Server::fdOpenFdFromDevice(Process *proc, const char *name, int flags, mode_t mode) {
	// Look for device by name.
	Device *device=devices.open(name);
	if (device==NULL)
		return -1;

	// Create and add FdEntry to list.
	int fd=this->fdCreate();
	if (fd==-1)
		return -1;
	FdEntry *entry=this->fdGet(fd);
	entry->type=FdTypeDevice;
	entry->refCount=1;
	entry->d.device=device;

	// Add internal fd to proc's list.
	int procFd=proc->fdAdd(entry->fd);
	if (procFd==-1) {
		// TODO: Remove above entry from entries list.
		return -1;
	}

	return procFd;
}

bool Server::fdClose(Process *proc, int fd) {
	// Find corressponding FdEntry.
	FdEntry *entry=this->fdGet(fd);
	if (entry==NULL)
		return false;

	// Will this call result in the FD being closed?
	assert(entry->refCount>0);
	if (entry->refCount<=1) {
		// Actually close the FD.
		switch(entry->type) {
			case FdTypeFd:
				if (close(entry->d.fd)==-1)
					return false;
			break;
			case FdTypeSocket:
				if (!entry->d.socket->close())
					return false;
			break;
			case FdTypeDevice:
				// TODO: Need to call devices->close(name), but don't have name.
			break;
			default:
				return false;
			break;
		}

		// Remove from our list of FDs.
		free(entry);
		fdEntries[fd]=NULL;
	}
	else
		--entry->refCount;

	// Remove from proc's list of FDs.
	proc->fdRemove(fd);

	return true;
}

Server::FdEntry *Server::fdGet(int fd) {
	return (fd>=0 && fd<fdEntries.size()) ? fdEntries[fd] : NULL;
}

int Server::fdCreate(void) {
	// Look for unused entries.
	size_t i;
	for(i=0;i<fdEntries.size();++i) {
		if (fdEntries[i]==NULL) {
			fdEntries[i]=(FdEntry *)malloc(sizeof(FdEntry));
			if (fdEntries[i]==NULL)
				return -1;
			fdEntries[i]->fd=i;
			fdEntries[i]->type=FdTypeNone;
		}
		if (fdEntries[i]->type==FdTypeNone)
			return i;
	}

	// Otherwise add new entry.
	FdEntry *entry=(FdEntry *)malloc(sizeof(FdEntry));
	if (entry==NULL)
		return -1;
	entry->fd=i;
	entry->type=FdTypeNone;
	fdEntries.push_back(entry);
	return i;
}