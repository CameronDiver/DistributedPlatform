#include <cstring>

#include "devzero.h"

DeviceZero::DeviceZero() {
}

DeviceZero::~DeviceZero() {
}

ssize_t DeviceZero::write(const void *data, size_t size) {
	return size; // /dev/zero accepts and discards all input.
}

ssize_t DeviceZero::read(void *data, size_t max) {
	// Produce continuous stream of NUL bytes.
	if (data!=NULL && max>0)
		memset(data, 0, max);
	return max;
}