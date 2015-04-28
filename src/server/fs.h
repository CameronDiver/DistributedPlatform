#ifndef FS_H
#define FS_H

class Fs {
 public:
 	virtual char *fileLocalPath(const char *path); // Returns a malloc'd string pointing to a local file with the same contents as the one pointed to by path.
};

#endif
