#include "../../stdlib/math.h"
#include "../../stdlib/stdio.h"
#include "../../stdlib/stdlib.h"
#include "../../stdlib/unistd.h"

int main(unsigned int argc, char **argv) {
	// Check arguments.
	if (argc>2)
	{
		Dprintf("Usage: %s [STRING]\n");
		return EXIT_FAILURE;
	}
	
	// Extract string to print.
	const char defaultStr[]="y";
	const char *str=defaultStr;
	if (argc==2)
		str=argv[1];
	
	// Print until killed.
	while(1)
		Dprintf("%s\n", str);
	
	return EXIT_SUCCESS;
}
