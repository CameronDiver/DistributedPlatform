#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fsdirect.h"

FSDirect::FSDirect() {
	containerPath=NULL;
}

FSDirect::~FSDirect(void) {
	free(containerPath);
	containerPath=NULL;
}

bool FSDirect::mountFile(const char *path) {
	// Check if already have a mounted file.
	if (containerPath!=NULL)
		return false;

	// Check path is a directory.
	struct stat s;
	if(stat(path,&s)!=0 || !(s.st_mode & S_IFDIR))
		return false;

	// Store path (and ensure is complete with a terminating '/').
	size_t pathLen=strlen(path);
	size_t pathSize=pathLen+1;
	if (path[pathLen-1]!='/')
		++pathSize;
	containerPath=(char *)malloc(pathSize);
	if (containerPath==NULL)
		return false;
	memcpy((void *)containerPath, (const void *)path, pathLen);
	containerPath[pathSize-2]='/';
	containerPath[pathSize-1]='\0';

	return true;
}

bool FSDirect::unmount() {
	free(containerPath);
	containerPath=NULL;
	return true;
}

char *FSDirect::fileLocalPath(const char *path) {
	// Find memory required.
	size_t containerLen=strlen(containerPath);
	size_t pathLen=strlen(path);
	size_t localLen=containerLen+pathLen;

	// Attempt to allocate memory.
	char *localPath=(char *)malloc(localLen+1);
	if (localPath==NULL)
		return NULL;

	// Copy data.
	memcpy((void *)localPath, (const void *)containerPath, containerLen);
	memcpy((void *)(localPath+containerLen), (const void *)path, pathLen);
	localPath[localLen]='\0';

	return localPath;
}