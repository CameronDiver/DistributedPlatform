#include "math.h"

double Dsqrt(double x)
{
	return Dsqrtl(x);
}

float Dsqrtf(float x)
{
	return Dsqrt(x);
}

long double Dsqrtl(long double x)
{
	// Initial guess.
	long double r=x/10;
	
	// Babylonian method.
	r=(r+x/r)/2;
	r=(r+x/r)/2;
	r=(r+x/r)/2;
	r=(r+x/r)/2;
	r=(r+x/r)/2;
	r=(r+x/r)/2;
	r=(r+x/r)/2;
	r=(r+x/r)/2;
	
	return r;
}
