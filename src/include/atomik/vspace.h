/*
 *    vspace.c: Page table, directory and frame system calls
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

#ifndef _ATOMIK_VSPACE_H
#define _ATOMIK_VSPACE_H

#include <stdio.h>

#define ATOMIK_PAGEATTR_READABLE   1
#define ATOMIK_PAGEATTR_WRITABLE   2
#define ATOMIK_PAGEATTR_EXECUTABLE 4
#define ATOMIK_PAGEATTR_KERNEL     8
#define ATOMIK_PAGEATTR_PRESENT    16

#define ATOMIK_INVALID_ADDR ((uintptr_t) -1)


/* These pointers are required in order to remap lowmem
 * addresses into kernel addresses.
 */

extern void  *atomik_free_start;
extern size_t atomik_free_size;

extern void  *atomik_remap_start;
extern size_t atomik_remap_size;

static inline void *
__atomik_phys_to_remap (uintptr_t addr)
{
  return (void *) (addr - (uintptr_t) atomik_free_start + (uintptr_t) atomik_remap_start);
}

static inline uintptr_t
__atomik_remap_to_phys (void *addr)
{
  return (uintptr_t) addr + (uintptr_t) atomik_free_start - (uintptr_t) atomik_remap_start;
}

static inline int
__atomik_phys_is_remappable (void *addr, size_t size)
{
  return ((uintptr_t) addr - (uintptr_t) atomik_free_start) +
        size <= atomik_remap_size;
}

/*
 * System calls
 */
int atomik_page_remap (capslot_t *, uint8_t);

int atomik_pd_map_pagetable (capslot_t *, capslot_t *, uintptr_t);

int atomik_pt_map_page (capslot_t *, capslot_t *, uintptr_t);

/*
 * Convenience functions
 */
uintptr_t capslot_vspace_resolve (capslot_t *, uintptr_t, uint8_t, error_t *);

#endif /* _ATOMIK_VSPACE_H */
