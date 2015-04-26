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
#include "../misc/syscommon.h"

static int callbackInc(void *user, int argc, char **argv, char **azColName){
   (*((int *)user))++;
   return 0;
}

extern "C" void serverSysCall(void *gdata, uint32_t id, ...)
{
	// Find pid.
	Server *server=(Server *)gdata;
	pid_t posixPID=getpid();
	ProcessPID pid=ProcessPIDError;
	unsigned int i;
	for(i=0;i<server->procs.size();++i)
		if (server->procs[i]->getPosixPID()==posixPID) {
			pid=i;
			break;
		}
	if (pid==ProcessPIDError) {
		log(LogLevelErr, "System call from unknown process with POSIX pid=%u.\n", (unsigned)posixPID);
		return;
	}

	// Call function within class to do rest of the work.
	va_list ap;
	va_start(ap, id);
	server->syscall(pid, id, ap);
	va_end(ap);
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

	// Free devices.
	devices.~Devices();

	return true;
}

void Server::stop(void) {
	log(LogLevelInfo, "Stop request received.\n");
	this->stopFlag=true;
}

ProcessPID Server::processFork(ProcessPID parentPID) {
	// Create child process.
	Process *parent=procs[parentPID];
	Process *child=parent->forkCopy(&serverSysCall, (void *)this);
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
  	return 0; // fork() returns 0 to child.
  }
  else
  	return childPID; // fork() return child pid to parent.
}

void Server::syscall(ProcessPID pid, int id, va_list ap) {
	Process *curr=this->procs[pid];

	// Parse system call.
	switch(id) {
		case SysCommonSysCallExit: {
			uint32_t status=(uint32_t)va_arg(ap, uint32_t);
			exit(status);
		} break;
		case SysCommonSysCallFork: {
			int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
			*ret=this->processFork(pid);
		} break;
		case SysCommonSysCallGetPid: {
			int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
			*ret=pid;
		} break;
		case SysCommonSysCallAlloc:	{
			// TODO: Take server wide maxRam into account.
			void **ret=(void **)va_arg(ap, void **);
			void *ptr=(void *)va_arg(ap, void *);
			size_t size=(size_t)va_arg(ap, size_t);
			*ret=realloc(ptr, size);
		} break;
		case SysCommonSysCallExec: {
			const char *path=(const char *)va_arg(ap, const char *);
			uint32_t argc=(uint32_t)va_arg(ap, uint32_t);
			char **argv=(char **)va_arg(ap, char **);

			// Create and load new process.
			Process *newProc=new Process;
			if (newProc->loadFileFS(this->filesystem, path))
			{
				// Copy environment.
				newProc->setEnviron(curr->getEnviron());

				// Exec should preserve current working directory.
				newProc->setCwd(curr->getCwd());

				// 'Unload' old process.
				curr->~Process();

				// Update process array.
				this->procs[pid]=newProc;

				// Run (without forking).
				newProc->arun(&serverSysCall, (void *)this, false, argc, (const char **)argv);
				exit(EXIT_SUCCESS);
			}
		} break;
		case SysCommonSysCallGetCwd: {
			uint32_t *ret=(uint32_t *)va_arg(ap, uint32_t *);
			char *buf=(char *)va_arg(ap, char *);
			uint32_t size=(uint32_t)va_arg(ap, uint32_t);

			// Check we have a current working directory.
			if (this->procs[pid]->getCwd()==NULL)
				*ret=0;
			else if (buf!=NULL) {
				// Copy string.
				const char *cwd=this->procs[pid]->getCwd();
				strncpy(buf, cwd, size);

				// Terminate with null byte in case full length.
				buf[size-1]='\0';

				// Indicate true length.
				*ret=strlen(cwd)+1;
			}
		} break;
		case SysCommonSysCallChDir: {
			int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
			char *path=(char *)va_arg(ap, char *);

			// TODO: Check path is sensible?

			*ret=(this->procs[pid]->setCwd(path) ? 0 : -1);
		} break;
		case SysCommonSysCallRead: {
			int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
			int32_t fd=(int32_t)va_arg(ap, int32_t);
			void *buf=(void *)va_arg(ap, void *);
			uint32_t count=(uint32_t)va_arg(ap, uint32_t);

			// TODO: Check permissions etc.
			*ret=this->fdRead(fd, buf, count);
		}
		break;
		case SysCommonSysCallWrite: {
			int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
			int32_t fd=(int32_t)va_arg(ap, int32_t);
			const void *buf=(const void *)va_arg(ap, const void *);
			uint32_t count=(uint32_t)va_arg(ap, uint32_t);

			// TODO: Check permissions etc.
			*ret=this->fdWrite(fd, buf, count);
		}
		break;
		case SysCommonSysCallOpen: {
			int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
			const char *pathname=(const char *)va_arg(ap, const void *);
			uint32_t flags=(uint32_t)va_arg(ap, uint32_t);
			uint32_t mode=(uint32_t)va_arg(ap, uint32_t);

			// TODO: Check permissions etc.
			// TODO: Path needs simplifying and made absolute (so if ./ make relative to process cwd)
			*ret=this->fdOpenFdFromPath(curr, pathname, mode, flags);
		}
		break;
		case SysCommonSysCallClose: {
			int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
			int32_t fd=(int32_t)va_arg(ap, int32_t);

			// TODO: Check permissions etc.
			*ret=this->fdClose(curr, fd);
		}
		break;
		default:
			log(LogLevelErr, "Invalid system call id %u.\n", id); // TODO: Give more details (such as process).
		break;
	}
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

bool Server::processRun(ProcessPID pid, bool doFork, unsigned int argc, ...) {
	assert(pid>=1 && pid<procs.size());
	
	// Run process.
	va_list ap;
	va_start(ap, argc);
	bool ret=procs[pid]->vrun(&serverSysCall, (void *)this, doFork, argc, ap);
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
			if (!this->processRun(shellPID, true, 1, "home/.profile"))
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

	return entry->fd;
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
	int pathFd=-1, fd=-1;
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
	proc->fds.push_back(fd);

	free(localPath);
	return entry->fd;

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
	proc->fds.push_back(fd);

	return entry->fd;
}

int Server::fdClose(Process *proc, int fd) {
	// Find corressponding FdEntry.
	FdEntry *entry=this->fdGet(fd);
	if (entry==NULL)
		return -1;

	// Will this call result in the FD being closed?
	assert(entry->refCount>0);
	if (entry->refCount<=1) {
		// Actually close the FD.
		switch(entry->type) {
			case FdTypeFd:
				if (close(entry->d.fd)==-1)
					return -1;
			break;
			case FdTypeSocket:
				if (!entry->d.socket->close())
					return -1;
			break;
			case FdTypeDevice:
				// TODO: Need to call devices->close(name), but don't have name.
			break;
			default:
				return -1;
			break;
		}

		// Remove from our list of FDs.
		free(entry);
		fdEntries[fd]=NULL;
	}
	else
		--entry->refCount;

	// Remove from proc's list of FDs.
	// TODO: this

	return 0;
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
			return (fdEntries[i]!=NULL ? i : -1);
		} else if (fdEntries[i]->type==FdTypeNone)
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