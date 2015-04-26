#ifndef DEVICE_H
#define DEVICE_H

#include <cstddef>
#include <sys/types.h>

class Device {
public:
	Device();
	virtual ~Device();

	virtual ssize_t write(const void *data, size_t size);
	virtual ssize_t read(void *data, size_t max);
};

#endif