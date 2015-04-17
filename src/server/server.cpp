#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include "net/sockettcp.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "server.h"

extern "C" void serverSysCall(void *gdata, uint32_t id, ...)
{
	// Find pid.
	Server *server=(Server *)gdata;
	pid_t posixPID=getpid();
	ProcessPID pid=ProcessPIDError;
	unsigned int i;
	for(i=0;i<server->procs.size();++i)
		if (server->procs[i].getPosixPID()==posixPID) {
			pid=i;
			break;
		}
	if (pid==ProcessPIDError)
		return;
	Process *curr=&server->procs[pid];
	
	// Parse system call.
	va_list ap;
	va_start(ap, id);
	switch(id)
	{
		case 0: // exit
		{
			uint32_t status=(uint32_t)va_arg(ap, uint32_t);
			exit(status);
		}
		break;
		case 1: // fork
		{
	  	int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
			*ret=server->processFork(pid);
	  }
		break;
		case 2: // getpid
	  {
	  	int32_t *ret=(int32_t *)va_arg(ap, int32_t *);
	  	*ret=pid;
	  }
		break;
		case 3: // alloc
		{
			// TODO: Take server wide maxRam into account.
			void **ret=(void **)va_arg(ap, void **);
			void *ptr=(void *)va_arg(ap, void *);
			size_t size=(size_t)va_arg(ap, size_t);
			*ret=realloc(ptr, size);
		}
		break;
		case 4: // exec
		{
			const char *path=(const char *)va_arg(ap, const char *);
			uint32_t argc=(uint32_t)va_arg(ap, uint32_t);
			char **argv=(char **)va_arg(ap, char **);
			
			// Create and load new process.
			Process *newProc=new Process;
			if (newProc->loadFileFS(server->filesystem, path))
			{
				// 'Unload' old process.
				curr->~Process();
				
				// Update process array.
				server->procs[pid]=*newProc;
				
				// Run (without forking).
				newProc->arun(&serverSysCall, (void *)server, false, argc, (const char **)argv);
				exit(EXIT_SUCCESS);
			}
		}
		break;
		default:
			// TODO: What to do in case of invalid syscall?
		break;
	}
	
	va_end(ap);
}

Server::Server(int port) {
	Process dummyProc; // To create PID of 1 for init.
	procs.push_back(dummyProc);
	
	tcpPort=port;
	tcpSockFd=-1;
	filesystem=NULL;
}

Server::~Server(void) {
	Server::tcpClose();
}

bool Server::run(FS *fs, const char *initPath) {
	// Setup file system.
	filesystem=fs;
	
	// Load init process.
	Process initProc;
	if (!initProc.loadFileFS(fs, initPath))
		return false;
	
	// Add to list of processes.
	ProcessPID initPID=Server::processAdd(&initProc);
	if (initPID==ProcessPIDError)
		return false;
	
	// Run init process (forking, so non-blocking).
	if (!this->processRun(initPID, true))
		return false;

	// Setup TCP socket for listening.
	if (!this->tcpListen(tcpPort))
		return false;

	// Check for any new TCP activity (infinite loop).
	this->tcpPoll();

	return true;
}

ProcessPID Server::processFork(ProcessPID parentPID) {
	// Create child process.
	Process *parent=&procs[parentPID];
	Process *child=parent->forkCopy(&serverSysCall, (void *)this);
	if (child==NULL)
		return ProcessPIDError;
	
	// Add child to list of processes.
	ProcessPID childPID=procs.size();
	procs.push_back(*child);
	
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
  	procs[childPID].setPosixPID(getpid());
  	return 0; // fork() returns 0 to child.
  }
  else
  	return childPID; // fork() return child pid to parent.
}

ProcessPID Server::processAdd(Process *proc) {
	// Check process is loaded but not running (hence ready to run).
	if (proc->getState()!=ProcessState::Loaded)
		return ProcessPIDError;
	
	// Add to queue.
	ProcessPID pid=procs.size();
	procs.push_back(*proc);
	
	return pid;
}

bool Server::processRun(ProcessPID pid, bool doFork, unsigned int argc, ...) {
	assert(pid>=1 && pid<procs.size());
	
	// Run process.
	va_list ap;
	va_start(ap, argc);
	bool ret=procs[pid].vrun(&serverSysCall, (void *)this, doFork, argc, ap);
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
	fd_set fdSetActive, fdSetRead;
	FD_ZERO(&fdSetActive);
	FD_SET(tcpSockFd, &fdSetActive);

	while(1) {
		// Block until input arrives on one or more active sockets.
		fdSetRead=fdSetActive;
		if (select(FD_SETSIZE, &fdSetRead, NULL, NULL, NULL)<0)
			break;

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
}

bool Server::tcpRead(Connection *con) {
	SocketTcp *sockTcp=dynamic_cast<SocketTcp *>(con->sock);
	int fd=sockTcp->sockFd;

	char buffer[1024];
	int nbytes=read(fd, buffer, 1024);
	if (nbytes<=0)
		return false; // EOF or error.

	log(LogLevelInfo, "Received message: '%s'.\n", buffer);

	return true;
}