#include "device.h"

Device::Device() {
}

Device::~Device() {
}

ssize_t Device::write(const void *data, size_t size) {
	return -1;
}

ssize_t Device::read(void *data, size_t max) {
	return -1;
}