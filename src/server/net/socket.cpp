#include "socket.h"

Socket::Socket() {
	dir=Socket::DirectionNone;
	type=Socket::TypeNone;
}

Socket::~Socket() {
	this->close();
}

ssize_t Socket::write(const void *data, size_t size) {
	return -1;
}

ssize_t Socket::read(void *data, size_t max) {
	return -1;
}

bool Socket::open(void) {
	return false;
}

bool Socket::close(void) {
	return false;
}