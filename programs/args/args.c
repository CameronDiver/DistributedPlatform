#include "../../stdlib/stdio.h"
#include "../../stdlib/stdlib.h"

int main(unsigned int argc, char **argv) {
	// TODO: This doesn't seem to want to use our own version of printf.
	printf("argc=%u:\n", argc);
	unsigned int i;
	for(i=0;i<argc;++i)
		printf("	%s\n", argv[i]);
	return EXIT_SUCCESS;
}
