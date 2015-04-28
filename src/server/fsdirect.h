#ifndef FSDIRECT_H
#define FSDIRECT_H

#include "fs.h"

class FsDirect : public Fs {
 public:
	FsDirect(); // Path to file container with the file system within.
	~FsDirect(void);

	bool mountFile(const char *path);
	bool unmount();

	char *fileLocalPath(const char *path); // See fs.h prototype of same function.
private:
	char *containerPath;
};

#endif
