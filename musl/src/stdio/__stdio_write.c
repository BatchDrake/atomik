
#ifdef __ATOMIK__
#include <arch.h>
#else
#include <arch/i386/atomik.h>
#include <atomik-user.h>
#endif

#include "stdio_impl.h"

size_t __stdio_write(FILE *f, const unsigned char *buf, size_t len)
{
#ifdef __ATOMIK__
  while (len--)
    __arch_debug_putchar (*buf++);
#else
  while (len--)
    d_putch (*buf++);
#endif
  return len;
}
