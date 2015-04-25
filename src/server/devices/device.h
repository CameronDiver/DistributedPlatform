#ifndef DEVICE_H
#define DEVICE_H

class Device {
public:
	Device();
	~Device();

	virtual ssize_t write(const void *data, size_t size);
	virtual ssize_t read(void *data, size_t max);
};

#endif