#include "connection.h"

Connection::Connection(Socket *socket) {
	sock=socket;
}

Connection::~Connection() {
}

ssize_t Connection::write(const void *data, size_t size) {
	return sock->write(data, size);
}

ssize_t Connection::read(void *data, size_t max) {
	return sock->read(data, max);
}