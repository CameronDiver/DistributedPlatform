#ifndef FSDIRECT_H
#define FSDIRECT_H

#include "fs.h"

class FSDirect : public FS {
 public:
	FSDirect(); // Path to file container with the file system within.
	~FSDirect(void);

	bool mountFile(const char *path);
	bool unmount();

	char *fileLocalPath(const char *path); // See fs.h prototype of same function.
private:
	char *containerPath;
};

#endif
