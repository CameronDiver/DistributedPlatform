#ifndef CONNECTION_H
#define CONNECTION_H

#include "net/socket.h"

class Connection {
 public:
 	Socket *sock;

	Connection(Socket *socket);
	~Connection();
 	ssize_t write(const void *data, size_t size);
 	ssize_t read(void *data, size_t max);
 private:
};

#endif