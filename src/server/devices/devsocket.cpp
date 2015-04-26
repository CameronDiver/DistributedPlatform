#include <cstring>

#include "devsocket.h"

DeviceSocket::DeviceSocket(Socket *socket) {
	sock=socket;
}

DeviceSocket::~DeviceSocket() {
}

ssize_t DeviceSocket::write(const void *data, size_t size) {
	if (sock==NULL)
		return -1;
	return sock->write(data, size);
}

ssize_t DeviceSocket::read(void *data, size_t max) {
	if (sock==NULL)
		return -1;
	return sock->read(data, max);
}