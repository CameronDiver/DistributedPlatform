#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	if (argc<=1)
	{
		Dprintf("Usage: %s NUMBER ...\n");
		return EXIT_FAILURE;
	}
	
	int i;
	for(i=1;i<argc;++i)
	{
		unsigned long long int n=atoll(argv[i]);
		Dprintf("%llu:", n);
		
		// Special case.
		if (n<2)
		{
			Dprintf(" %llu\n", n);
			continue;
		}
		
		// Simple trial division.
		unsigned long long int sqrtn=sqrt(n);
		unsigned long long int d;
		for(d=2;d<=sqrtn;++d)
		{
			unsigned long long int q=n/d;
			unsigned long long int r=n-d*(n/d);
			if (r==0)
			{
				// Print factor.
				Dprintf(" %llu", d);
				
				// Update n and sqrt n.
				n=q;
				sqrtn=sqrt(n);
				
			  // Test this value of d, again may be another factor.
				--d;
			}
		}
		
		// No factors?
		if (n!=1)
			Dprintf(" %llu", n);
		
		Dprintf("\n");
	}
	
	return EXIT_SUCCESS;
}
