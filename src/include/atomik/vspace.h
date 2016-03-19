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
#include <machinedefs.h>

#define ATOMIK_PAGEATTR_READABLE   1
#define ATOMIK_PAGEATTR_WRITABLE   2
#define ATOMIK_PAGEATTR_EXECUTABLE 4
#define ATOMIK_PAGEATTR_KERNEL     8
#define ATOMIK_PAGEATTR_PRESENT    16

#define ATOMIK_INVALID_ADDR ((uintptr_t) -1)

struct vremap
{
  uintptr_t virt_start;
  uintptr_t phys_start;
  size_t    virt_max;
  size_t    virt_len;
  size_t    virt_pages;
};

typedef struct vremap vremap_t;

typedef uint8_t ptbuf_t[PT_SIZE];
/*
 * These pointers are required in order to remap lowmem
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

static inline uintptr_t
__atomik_capslot_get_page_vaddr (const capslot_t *page)
{
  return page->page.vaddr_lo1 +
        (page->page.vaddr_lo2 << 8) +
        (page->page.vaddr_hi << 16);
}

static inline void
__atomik_capslot_set_page_vaddr (capslot_t *page, uintptr_t vaddr)
{
  page->page.vaddr_lo1 = vaddr & 0xff;
  page->page.vaddr_lo2 = (vaddr >> 8) & 0xff;
  page->page.vaddr_hi  = vaddr >> 16;
}

static inline uint8_t
__atomik_access_to_page_attr (uint8_t access)
{
  uint8_t attr = ATOMIK_PAGEATTR_PRESENT;

  if (access & ATOMIK_ACCESS_EXEC)
      attr |= ATOMIK_PAGEATTR_EXECUTABLE;

  if (access & ATOMIK_ACCESS_READ)
      attr |= ATOMIK_PAGEATTR_READABLE;

  if (access & ATOMIK_ACCESS_WRITE)
      attr |= ATOMIK_PAGEATTR_WRITABLE;

  return attr;
}

static inline uint8_t
__atomik_capslot_to_page_attr (const capslot_t *page)
{
  return __atomik_access_to_page_attr (page->page.access);
}
/*
 * VRemap API
 */

void *vremap_translate (const vremap_t *, uintptr_t, size_t);

int  vremap_remap (vremap_t *, uintptr_t, size_t);

int vremap_alloc (vremap_t *, size_t);

/*
 * System calls
 */
int atomik_page_remap (capslot_t *, uint8_t);

int atomik_pt_remap (capslot_t *pt, uint8_t);

int atomik_pd_map_pagetable (capslot_t *, capslot_t *, uintptr_t);

int atomik_pt_map_page (capslot_t *, capslot_t *, uintptr_t);

/*
 * Convenience functions
 */
uintptr_t capslot_vspace_resolve (capslot_t *, uintptr_t, uint8_t, error_t *);

int capslot_vspace_switch (capslot_t *);

#endif /* _ATOMIK_VSPACE_H */
