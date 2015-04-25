#ifndef SOCKET_H
#define SOCKET_H

#include <cstddef>
#include <sys/types.h>

class Socket {
 public:
	enum Direction {
		DirectionNone=0,
		DirectionIn=1,
		DirectionOut=2,
		DirectionInOut=DirectionIn|DirectionOut,
	};

	enum Type {
		TypeNone,
		TypeTcp,
	};

 	Direction dir;
 	Type type;

 	Socket();
 	~Socket();
 	virtual ssize_t write(const void *data, size_t size);
 	virtual ssize_t read(void *data, size_t max);
	virtual bool open(void);
	virtual bool close(void);
};

#endif