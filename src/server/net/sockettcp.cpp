#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "sockettcp.h"

int wrapperConnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	return connect(sockfd, addr, addrlen);
}

ssize_t wrapperRead(int fd, void *buf, size_t count) {
	return read(fd, buf, count);
}

ssize_t wrapperWrite(int fd, const void *buf, size_t count) {
	return write(fd, buf, count);
}

int wrapperClose(int fd) {
	return close(fd);
}

SocketTcp::SocketTcp() {
	state=SocketTcp::StateNone;
	type=Socket::TypeTcp;
}

SocketTcp::~SocketTcp() {
	// Ensure we disconnect.
	this->close();
}

ssize_t SocketTcp::write(const void *data, size_t size) {
	return (state!=SocketTcp::StateNone ? wrapperWrite(sockFd, data, size) : -1);
}

ssize_t SocketTcp::read(void *data, size_t max) {
	//need to make this read non-blocking?
	return (state!=SocketTcp::StateNone ? wrapperRead(sockFd, data, max) : -1);
}

bool SocketTcp::open(void) {
	return true;
}

bool SocketTcp::connect(const char *addrStr, unsigned int port) {
	// Check we are not already connected.
	if (state!=SocketTcp::StateNone)
		return false;

	sockFd=socket(AF_INET, SOCK_STREAM, 0);
	if(sockFd<0)
		return false;

	server=gethostbyname(addrStr);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family=AF_INET;
	memcpy((void *)&addr.sin_addr.s_addr, (void *)server->h_addr, server->h_length);
	addr.sin_port=htons(port);

	if (wrapperConnect(sockFd, (struct sockaddr *)&addr, sizeof(addr))<0) 
		return false;

	// Update state.
	state=SocketTcp::StateClient;
	dir=Socket::DirectionInOut;

	return true;
}

bool SocketTcp::listen(const struct sockaddr_in *aaddr, socklen_t aaddrLen, int asockFd) {
	if (state!=SocketTcp::StateNone)
		return false;

	addr=*aaddr;
	addrLen=aaddrLen;
	sockFd=asockFd;

	// Update state.
	state=SocketTcp::StateServer;
	dir=Socket::DirectionInOut;

	return true;
}

bool SocketTcp::close(void) {
	if (state==SocketTcp::StateNone)
		return false;

	if (wrapperClose(sockFd)==-1)
		return false;

	// Update state.
	state=SocketTcp::StateServer;
	dir=Socket::DirectionNone;

	return true;
}