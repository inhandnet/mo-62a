#include <stdio.h>

#ifndef VERSION
#define VERSION "unknown"
#endif

#ifndef BUILD_DATE
#define BUILD_DATE "unknown"
#endif

int main(void)
{
	printf("MO-62A %s\nBuilt:  %s\n", VERSION, BUILD_DATE);
	return 0;
}
