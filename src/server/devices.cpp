#include <cstring>
#include <cstdlib>

#include "devices.h"
#include "log.h"

Devices::Devices() {
}

Devices::~Devices() {
	// Remove all devices.
	size_t i=0;
	while(i<entries.size()) {
		if (!this->remove(entries[i].name)) {
			log(LogLevelErr, "Could not remove device at '/dev/%s'.", entries[i].name);
			++i;
		}
	}
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

	// Create (empty) file to represent this device in /dev.
	// TODO: this

	return true;
}

bool Devices::remove(const char *name) {
	// Grab entry.
	if (name==NULL)
		return false;
	ssize_t index=this->getIndex(name);
	if (index==-1)
		return false;
	Entry *entry=&(entries[index]);

	// Still in use?
	if (entry->refCount>0)
		return false;

	// Tidy up.
	free(entry->name);
	entry->name=NULL;
	delete entry->device;
	entries.erase(entries.begin()+index);
	// TODO: Remove file from /dev.

	return true;
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

	--entry->refCount;
	return true;
}

Devices::Entry *Devices::get(const char *name) {
	ssize_t index=this->getIndex(name);
	return (index!=-1 ? &(entries[index]) : NULL);
}

ssize_t Devices::getIndex(const char *name) {
	size_t i;
	for(i=0;i<entries.size();++i)
		if (entries[i].name!=NULL && !strcmp(entries[i].name, name))
			return i;
	return -1;
}