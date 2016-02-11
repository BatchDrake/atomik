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

#include <stdio.h>
#include <arch.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <atomik/atomik.h>
#include <atomik/cap.h>
#include <atomik/vspace.h>

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

static uint8_t
__atomik_capslot_to_page_attr (const capslot_t *page)
{
  uint8_t attr = ATOMIK_PAGEATTR_PRESENT;

  if (page->page.access & ATOMIK_ACCESS_EXEC)
      attr |= ATOMIK_PAGEATTR_EXECUTABLE;

  if (page->page.access & ATOMIK_ACCESS_READ)
      attr |= ATOMIK_PAGEATTR_READABLE;

  if (page->page.access & ATOMIK_ACCESS_WRITE)
      attr |= ATOMIK_PAGEATTR_WRITABLE;

  return attr;
}

/* This is *only* used to update attributes */
int
atomik_page_remap (capslot_t *page, uint8_t attr)
{
  error_t exception = ATOMIK_SUCCESS;

  if (page->object_type != ATOMIK_OBJTYPE_PAGE)
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  if (page->page.pt == NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_MAP_FIRST);

  /* TODO: Write */
  return 0;

fail:
  return -exception;
}

int
atomik_pd_map_pagetable (capslot_t *pd, capslot_t *pt, uintptr_t vaddr)
{
  error_t exception = ATOMIK_SUCCESS;

  if (pd->object_type != ATOMIK_OBJTYPE_PD)
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  if (pt->object_type != ATOMIK_OBJTYPE_PT)
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  /* Cannot map twice the same pagetable */
  if (pt->pt.pd != NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_UNMAP_FIRST);

  pt->pt.pd = pd;

  __atomik_capslot_set_page_vaddr (pt, vaddr);

  __arch_map_pagetable (pd->pd.base, /* Remap'd addr */
                        pt->pt.base, /* Remap'd addr */
                        vaddr,
                        __atomik_capslot_to_page_attr (pt));

  /* TODO: Write */
  return 0;

fail:
  return -exception;
}

int
atomik_pt_map_page (capslot_t *pt, capslot_t *page, uintptr_t vaddr)
{
  error_t exception = ATOMIK_SUCCESS;

  if (pt->object_type != ATOMIK_OBJTYPE_PT)
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  if (page->object_type != ATOMIK_OBJTYPE_PAGE)
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  /* Cannot map twice the same page */
  if (page->page.pt != NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_UNMAP_FIRST);

  page->page.pt = pt;

  __atomik_capslot_set_page_vaddr (pt, vaddr);

  __arch_map_pagetable (pt->pt.base,     /* Remap'd addr */
                        page->page.base, /* Remap'd addr */
                        vaddr,
                        __atomik_capslot_to_page_attr (page));

  /* TODO: Write */
  return 0;

fail:
  return -exception;
}

uintptr_t
capslot_vspace_resolve (capslot_t *pd, uintptr_t vaddr, uint8_t access, error_t *error)
{
  uintptr_t page_resolved;

  if ((page_resolved =
      __arch_resolve_page (pd->pd.base, PAGE_START (vaddr), access, error)) ==
          ATOMIK_INVALID_ADDR)
    return ATOMIK_INVALID_ADDR;

  return page_resolved | PAGE_OFFSET (vaddr);
}

