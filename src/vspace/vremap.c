/*
 *    vremap.c: Virtual remappings
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

#include <stdio.h>
#include <arch.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <atomik/atomik.h>
#include <atomik/cap.h>
#include <atomik/vspace.h>

#define KERNEL_VREMAP_PT_NUM __UNITS (KERNEL_VREMAP_MAX, PTRANGE_SIZE)

/* Used to know where to place the next vremap */
static size_t vremap_watermark = 0;

/* We need to preallocate these */
static ptbuf_t vremap_pts[KERNEL_VREMAP_PT_NUM] __attribute__ ((aligned (PT_SIZE)));

/* Current PD */
extern uintptr_t *curr_vspace;

/* Transform address */
void *
vremap_translate (const vremap_t *vremap, uintptr_t phys, size_t size)
{
  /* Does this object fit? */
  if (phys < vremap->phys_start ||
      phys + size > vremap->phys_start + vremap->virt_len)
    return NULL;

  /* Kernel remap starts somewhere else */
  ATOMIK_ASSERT (!vremap->force || !vremap->is_remap);
  
  if (vremap->is_remap)
    return __atomik_phys_to_remap (phys);

  return (void *) (phys - vremap->phys_start + vremap->virt_start);
}

/* Accepts bytes, automatically refreshes the TLB */
int
vremap_remap (vremap_t *vremap, uintptr_t phys, size_t size)
{
  uintptr_t pt, pt_prev;
  uintptr_t page;
  unsigned int i;
  
  /* Does it fit? */
  if (size > vremap->virt_max)
    return -1;

  vremap->phys_start = PAGE_START (phys);
  vremap->virt_len   = size + PAGE_OFFSET (phys);
  vremap->virt_pages = __UNITS (vremap->virt_len, PAGE_SIZE);
  vremap->is_remap   = !vremap->force &&
    __atomik_phys_is_remappable ((void *) phys, size);

  /* If this physical address is outside the kernel
   * remap we must update microkernel pages */
  if (!vremap->is_remap)
  {
    pt_prev = -1; /* We will never reach this value */
  
    for (i = 0; i < vremap->virt_pages; ++i)
    {
      page = vremap->virt_start + PAGE_ADDRESS (i);
      pt = VADDR_GET_PDE_INDEX (page - KERNEL_VREMAP_BASE);

      if (pt != pt_prev)
      {
        /* Ensure we have mapped this PT */
        __arch_map_pagetable (
          curr_vspace,
          &vremap_pts[pt],
          page,
          ATOMIK_PAGEATTR_WRITABLE |
          ATOMIK_PAGEATTR_PRESENT  |
          ATOMIK_PAGEATTR_KERNEL);

        pt_prev = pt;
      }

      /* Map page */
      __arch_map_page (
        &vremap_pts[pt],
        (void *) (vremap->phys_start + PAGE_ADDRESS (i)),
        page,
        ATOMIK_PAGEATTR_WRITABLE |
        ATOMIK_PAGEATTR_PRESENT  |
        ATOMIK_PAGEATTR_KERNEL);

      /* Invalidate */
      __arch_invalidate_page ((void *) page);
    }
  }
  
  return 0;
}

int
vremap_alloc_ex (vremap_t *vremap, size_t size, int force)
{
  size_t aligned_size;

  aligned_size = __ALIGN (size, PAGE_SIZE);

  if (vremap_watermark + aligned_size > KERNEL_VREMAP_MAX)
    return -1;

  vremap->virt_start = KERNEL_VREMAP_BASE + vremap_watermark;
  vremap->virt_max   = size;
  vremap->virt_len   = 0;
  vremap->virt_pages = 0;
  vremap->force      = force;
  vremap_watermark  += aligned_size;
  
  return 0;
}

int
vremap_alloc (vremap_t *vremap, size_t size)
{
  return vremap_alloc_ex (vremap, size, 0);
}
