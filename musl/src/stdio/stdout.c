#include "stdio_impl.h"

static unsigned char buf[UNGET];
static FILE f = {
	.buf = buf+UNGET,
	.buf_size = 0,
	.fd = 1,
	.flags = F_PERM | F_NORD,
	.lbf = -1,
	.write = __stdio_write,
	.seek = NULL,
	.close = NULL,
	.lock = -1,
};
FILE *const stdout = &f;
FILE *volatile __stdout_used = &f;
