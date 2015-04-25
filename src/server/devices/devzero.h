#ifndef DEVZERO_H
#define DEVZERO_H

#include "device.h"

class DeviceZero : public Device {
public:
	DeviceZero();
	~DeviceZero();

	ssize_t write(const void *data, size_t size);
	ssize_t read(void *data, size_t max);
};

#endif