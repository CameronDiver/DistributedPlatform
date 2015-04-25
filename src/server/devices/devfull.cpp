#include <cstring>

#include "devfull.h"

DeviceFull::DeviceFull() {
}

DeviceFull::~DeviceFull() {
}

ssize_t DeviceFull::write(const void *data, size_t size) {
	// TODO: Indicate disk full.
	return -1;
}

ssize_t DeviceFull::read(void *data, size_t max) {
	// Produce continuous stream of NUL bytes.
	if (data!=NULL && max>0)
		memset(data, 0, max);
	return max;
}

bool DeviceFull::open(void) {
	return true;
}

bool DeviceFull::close(void) {
	return true;
}