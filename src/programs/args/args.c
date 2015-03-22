#include "../../stdlib/stdio.h"
#include "../../stdlib/stdlib.h"
#include "../../stdlib/sys.h"

int main(unsigned int argc, char **argv) {
	Dprintf("PID=%u\n", (unsigned int)getpid());
	Dprintf("argc=%u:\n", argc);
	unsigned int i;
	for(i=0;i<argc;++i)
		Dprintf("	%s\n", argv[i]);
	return EXIT_SUCCESS;
}
