#ifndef DEVSOCKET_H
#define DEVSOCKET_H

#include "device.h"
#include "../net/socket.h"

class DeviceSocket : public Device {
public:
	DeviceSocket(Socket *socket);
	~DeviceSocket();

	ssize_t write(const void *data, size_t size);
	ssize_t read(void *data, size_t max);
private:
	Socket *sock;
};

#endif