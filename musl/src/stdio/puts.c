#include "stdio_impl.h"

int puts(const char *s)
{
	int r;
	
	(void) fputs (s, stdout);
        putc ('\n', stdout);
	
	return r;
}
