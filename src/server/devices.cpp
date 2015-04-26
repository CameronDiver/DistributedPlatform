#include <cstring>
#include <cstdlib>

#include "devices.h"

Devices::Devices() {
}

Devices::~Devices() {
}

bool Devices::add(const char *name, Device *device) {
	// Device with this name already exists?
	if (this->exists(name))
		return false;

	// Invalid device?
	if (device==NULL)
		return false;

	// Add to list.
	Entry entry;
	size_t nameSize=strlen(name)+1;
	entry.name=(char *)malloc(nameSize);
	if (entry.name==NULL)
		return false;
	memcpy(entry.name, name, nameSize);
	entry.device=device;
	entry.refCount=0;
	entries.push_back(entry);

	return true;
}

bool Devices::remove(const char *name) {
	// Grab entry.
	Entry *entry=this->get(name);
	if (entry==NULL)
		return false;

	// TODO: this
	return false;
}

bool Devices::exists(const char *name) {
	return (this->get(name)!=NULL);
}

Device *Devices::open(const char *name) {
	// Grab entry.
	Entry *entry=this->get(name);
	if (entry==NULL)
		return NULL;

	// Inc ref counter.
	++entry->refCount;

	return entry->device;
}

bool Devices::close(const char *name) {
	// Grab entry.
	Entry *entry=this->get(name);
	if (entry==NULL)
		return false;

	// Already closed?
	if (entry->refCount<=0)
		return false;

	// Nothing to be done?
	if (--entry->refCount>0)
		return true;

	// Actually close this device.
	// TODO: this
	free(entry->name);
	entry->name=NULL;

	return true;
}

Devices::Entry *Devices::get(const char *name) {
	size_t i;
	for(i=0;i<entries.size();++i)
		if (entries[i].name!=NULL && !strcmp(entries[i].name, name))
			return &(entries[i]);
	return NULL;
}