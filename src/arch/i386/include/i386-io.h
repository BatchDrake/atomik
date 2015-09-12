#ifndef _ARCH_I386_IO_H
#define _ARCH_I386_IO_H

/* inportb: Devuelve un byte del puerto de software PORT */
static inline uint8_t
inportb (uint16_t port)
{
  uint8_t rv;
  __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
  return rv;
}

/* outportb: EnvÃÂ­a un byte DATA al puerto de software PORT */
static inline void
outportb (uint16_t port, uint8_t data)
{
  __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}


static inline void
outl (uint16_t port, uint32_t value)
{
  __asm__ __volatile__("outl %1, %0" : : "dN" (port), "a" (value));
}

static inline uint32_t
inl (uint16_t port)
{
  uint32_t Ret;
  __asm__ __volatile__("inl %1, %0" : "=a" (Ret) : "dN" (port));
  return Ret;
}

#endif /* _ARCH_I386_IO_H */
