#include "devnull.h"

DeviceNull::DeviceNull() {
}

DeviceNull::~DeviceNull() {
}

ssize_t DeviceNull::write(const void *data, size_t size) {
	return size; // /dev/null accepts and discards all input.
}

ssize_t DeviceNull::read(void *data, size_t max) {
	return 0; // EOF.
}