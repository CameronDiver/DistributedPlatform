#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int main(unsigned int argc, char **argv) {
	char **ptr;
	for(ptr=environ;*ptr!=NULL;++ptr)
		puts(*ptr);
	return EXIT_SUCCESS;
}
