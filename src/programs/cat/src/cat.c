#include <stdio.h>
#include <stdlib.h>
#include <sys.h>

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2

#define BUFSIZE 128

int main(int argc, char **argv) {
	// If given a series of paths, read from those.
	if (argc>1) {
		int i;
		for(i=1;i<argc;++i) {
			int fd=open(argv[i], O_RDONLY, 0);
			if (fd==-1)
				continue;
			char buf[BUFSIZE];
			ssize_t count;
			while((count=read(fd, buf, BUFSIZE-1))>0) {
				buf[count]='\0';
				printf("%s", buf);
			}
			close(fd);
		}
	}

	// Otherwise read from stdin.
	// TODO: this

	return EXIT_SUCCESS;
}
