#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
	printf("PID=%u\n", (unsigned int)getpid());
	printf("argc=%u:\n", argc);
	unsigned int i;
	for(i=0;i<argc;++i)
		printf("	%s\n", argv[i]);
	return EXIT_SUCCESS;
}
