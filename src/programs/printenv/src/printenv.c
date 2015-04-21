#include <stdio.h>
#include <stdlib.h>

extern char **Denviron;

int main(int argc, char **argv) {
	char **ptr;
	for(ptr=Denviron;*ptr!=NULL;++ptr)
		Dprintf("%s\n", *ptr);
	return EXIT_SUCCESS;
}
