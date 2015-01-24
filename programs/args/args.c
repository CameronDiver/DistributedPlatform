#include "../../stdlib/stdio.h"
#include "../../stdlib/stdlib.h"

int main(unsigned int argc, char **argv) {
	Dprintf("argc=%u:\n", argc);
	unsigned int i;
	for(i=0;i<argc;++i)
		Dprintf("	%s\n", argv[i]);
	return EXIT_SUCCESS;
}
