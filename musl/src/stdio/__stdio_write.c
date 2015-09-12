
#include <arch.h>

#include "stdio_impl.h"

size_t __stdio_write(FILE *f, const unsigned char *buf, size_t len)
{
  while (len--)
    __arch_debug_putchar (*buf++);

  return len;
}
