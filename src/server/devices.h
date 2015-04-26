#ifndef DEVICES_H
#define DEVICES_H

#include <sys/types.h>
#include <vector>

#include "devices/device.h"

class Devices {
public:
	Devices();
	~Devices();

	bool add(const char *name, Device *device);
	bool remove(const char *name);
	bool exists(const char *name);
	Device *open(const char *name);
	bool close(const char *name);
private:
	typedef struct {
		char *name;
		Device *device;
		int refCount;
	}Entry;

	std::vector<Entry> entries;

	Entry *get(const char *name);
	ssize_t getIndex(const char *name);
};

#endif