#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	char **ptr;
	for(ptr=environ;*ptr!=NULL;++ptr)
		printf("%s\n", *ptr);
	return EXIT_SUCCESS;
}
