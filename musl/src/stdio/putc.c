#include "stdio_impl.h"

int putc(int c, FILE *f)
{
  const unsigned char buf[] = {c};

  f->write (f, buf, 1);
  
  return c;
}

weak_alias(putc, _IO_putc);
