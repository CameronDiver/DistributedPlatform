#ifndef DEVNULL_H
#define DEVNULL_H

#include "device.h"

class DeviceNull : public Device {
public:
	DeviceNull();
	~DeviceNull();

	ssize_t write(const void *data, size_t size);
	ssize_t read(void *data, size_t max);
};

#endif