#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	// Check arguments.
	if (argc>2)
	{
		printf("Usage: %s [STRING]\n");
		return EXIT_FAILURE;
	}
	
	// Extract string to print.
	const char defaultStr[]="y";
	const char *str=defaultStr;
	if (argc==2)
		str=argv[1];
	
	// Print until killed.
	while(1)
		printf("%s\n", str);
	
	return EXIT_SUCCESS;
}
