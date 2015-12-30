/*
 *    util.h: Some useful utility functions
 *    Copyright (C) 2014  Gonzalo J. Carracedo
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

#ifndef _UTIL_H
#define _UTIL_H

#define _JOIN(x, y) x ## y
#define JOIN(x, y) _JOIN (x, y)

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define CPPASSERT(expr) \
  typedef char JOIN (JOIN (__compiler_assert_, __FILE_ID), \
                     __LINE__) [2 * !!(expr) - 1]

# ifdef __GNUC__
#  define PACKED            __attribute__ ((packed))
#  define ALIGNED(x)        __attribute__ ((aligned (x)))
#  define PACKED_ALIGNED(x) __attribute__ ((packed, aligned (x)))
#  define COMPILER_APPEND   "gcc timestamp: " __TIMESTAMP__
# else
#  error "Unsupported compiler! (only GCC currently supported)"
# endif

# define __UNITS(x, wrdsiz) ((((x) + (wrdsiz - 1)) / wrdsiz))
# define __ALIGN(x, wrdsiz) (__UNITS(x, wrdsiz) * wrdsiz)

/* Page-related macros */
#define PAGE_SIZE            (1 << PAGE_BITS)
#define PAGE_MASK            (~(PAGE_SIZE - 1))
#define PAGE_ENTRIES         (PAGE_SIZE / sizeof (uintptr_t))
#define PAGE_START(x)        ((x) & PAGE_MASK)
#define CONTROL_BITS         (~PAGE_MASK)

#define PAGE_TABLE(addr)     (( (uintptr_t) addr) >> (VIRT_ADDR_BITS - PAGE_BITS)))
#define PAGE_ENTRY(addr)     ((((uintptr_t) addr) >> PAGE_BITS) & (PAGE_ENTRIES - 1))

#define ATOMIK_ASSERT(expr)                                             \
  if (!(expr))                                                          \
  {                                                                     \
    printf ("atomik: assertion failed in %s:%d\n"                       \
            "atomik: condition \"" STRINGIFY (expr) "\" not met\n");    \
    __arch_machine_halt ();                                             \
  }

#endif /* _UTIL_H */
