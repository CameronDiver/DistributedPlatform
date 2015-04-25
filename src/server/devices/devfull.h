#ifndef DEVFULL_H
#define DEVFULL_H

#include "device.h"

class DeviceFull : public Device {
public:
	DeviceFull();
	~DeviceFull();

	ssize_t write(const void *data, size_t size);
	ssize_t read(void *data, size_t max);
};

#endif