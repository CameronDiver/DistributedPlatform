#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	int i;
	bool space=false;
	for(i=1;i<argc;++i) {
		if (argv[i][0]=='\0')
			continue;
		if (space)
			printf(" ");
		printf("%s", argv[i]);
		space=true;
	}

	printf("\n");

	return EXIT_SUCCESS;
}