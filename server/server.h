#ifndef SERVER_H
#define SERVER_H

#include <cstddef>

#include "fs.h"

class Server {
 public:
	Server(size_t maxRam=128*1024*1024, size_t maxCores=1);
	~Server(void);

	bool run(FS *fs, const char *initPath);
};

#endif 
