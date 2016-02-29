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

#include <machinedefs.h>

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

/* Consistency checks. If this breaks, the page table/directory
 * layout is not properly defined in arch/XXX/machinedefs.h */
CPPASSERT (PAGE_BITS + PTE_BITS + PDE_BITS == VIRT_ADDR_BITS);

/* Page-related macros */
#define BIT(x)               (1 << (x))
#define PAGE_SIZE            BIT (PAGE_BITS)
#define PTRANGE_SIZE         BIT (PAGE_BITS + PTE_BITS)
#define PAGE_CONTROL_MASK    (PAGE_SIZE - 1)
#define PAGE_MASK            (~PAGE_CONTROL_MASK)
#define PTE_COUNT            BIT (PTE_BITS)
#define PDE_COUNT            BIT (PDE_BITS)
#define PTE_MASK             (PTE_COUNT - 1) /* After shift */
#define PDE_MASK             (PDE_COUNT - 1) /* After shift */
#define PAGE_START(x)        ((x) & PAGE_MASK)
#define PT_START(x)          ((x) & ~(BIT (PAGE_BITS + PTE_BITS) - 1))

#define PAGE_OFFSET(x)       ((x) & PAGE_CONTROL_MASK)
#define PAGE_NUMBER(x)       ((x) >> PAGE_BITS)
#define PAGE_ADDRESS(x)      ((x) << PAGE_BITS)

#define VADDR_GET_PDE_INDEX(addr)  \
  ((((uintptr_t) addr) >> (PAGE_BITS + PTE_BITS)) & PDE_MASK)
#define VADDR_GET_PTE_INDEX(addr)  \
  ((((uintptr_t) addr) >> PAGE_BITS) & PTE_MASK)

#define ATOMIK_ASSERT(expr)                                             \
  if (!(expr))                                                          \
  {                                                                     \
    printf ("atomik: assertion failed in %s:%d\n"                       \
            "atomik: condition \"" STRINGIFY (expr) "\" not met\n",     \
            __FILE__, __LINE__);                                        \
    __arch_machine_halt ();                                             \
  }

/*
 * Bittree macros
 */
#define BITTREE_DEBRUIJN32 0x77cb531
#define BITTREE_FULL_NODE  0xffffffff

/*
 * Bittree functions
 */

unsigned int bittree_find (uint32_t *, size_t);

void bittree_mark (uint32_t *, size_t, unsigned int);

void bittree_unmark (uint32_t *, size_t, unsigned int);

/*
 * General utility functions
 */
const char *error_to_string (error_t);

#endif /* _UTIL_H */
