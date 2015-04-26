#ifndef DEVICES_H
#define DEVICES_H

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
};

#endif