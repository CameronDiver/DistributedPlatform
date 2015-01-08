#include <cstring>

#include "util.h"

const char *strstrrev(const char *haystack, const char *needle) {
	if (*needle=='\0')
		return haystack;

	const char *result=NULL;
	for (;;) {
		const char *p=strstr(haystack, needle);
		if (p==NULL)
			break;
		result=p;
		haystack=p+1;
	}

	return result;
}
