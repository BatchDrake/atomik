/*
 *    i386-io.h: Input/Output functions for i386
 *    Copyright (C) 2016  Gonzalo J. Carracedo
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

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

/* outportb: Envía un byte DATA al puerto de software PORT */
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
