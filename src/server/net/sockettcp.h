#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include <cstddef>
#include <netdb.h>
#include <netinet/in.h>

#include "socket.h"

class SocketTcp : public Socket {
public:
	enum State { StateNone, StateClient, StateServer };

	SocketTcp::State state;
	int sockFd;
	struct sockaddr_in addr;
	socklen_t addrLen;
	struct hostent *server;

	SocketTcp();
	~SocketTcp();
 	ssize_t write(const void *data, size_t size);
 	ssize_t read(void *data, size_t max);

 	bool connect(const char *addr, unsigned int port);
 	bool listen(const struct sockaddr_in *aaddr, socklen_t aaddrLen, int asockFd); // Assumes already accept()'ed connection.
 	void disconnect(void);
private:
};

#endif