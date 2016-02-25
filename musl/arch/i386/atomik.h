/*
 *    atomik.h: Atomik system call interface
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

#ifndef _ARCH_I386_ATOMIK_H
#define _ARCH_I386_ATOMIK_H

#define SYS_CALL "int $0xa0"


static inline int
syscall_0 (unsigned long n)
{
  int r;

  __asm__ __volatile__ ("int $0xa0" :
                        "=a" (r) :
                        "a" (n));

  return r;
}

static inline int
syscall_1 (unsigned long n, unsigned long a0)
{
  int r;

  __asm__ __volatile__ ("int $0xa0" :
                          "=a" (r) :
                          "a" (n), "b" (a0));

  return r;
}

#endif /* _ARCH_I386_ATOMIK_H */
