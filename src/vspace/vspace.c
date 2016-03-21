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

uintptr_t *curr_vspace; /* If NULL: boot vspace */

/* This is *only* used to update page attributes. Allows to temporarily
 * drop permissions on page without creating extra capabilities.
 */
int
atomik_page_remap (capslot_t *page, uint8_t attr)
{
  error_t exception = ATOMIK_SUCCESS;

  if (page->object_type != ATOMIK_OBJTYPE_PAGE)
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  if (page->page.pt == NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_MAP_FIRST);

  __arch_map_page (page->page.pt->pt.base,
                   page->page.base,
                   __atomik_capslot_get_page_vaddr (page),
                   (__atomik_access_to_page_attr (page->page.access) & attr) |
                     ATOMIK_PAGEATTR_PRESENT);

  return 0;

fail:
  return -exception;
}

/* Equivalent thing at PT level
 */
int
atomik_pt_remap (capslot_t *pt, uint8_t attr)
{
  error_t exception = ATOMIK_SUCCESS;

  if (pt->object_type != ATOMIK_OBJTYPE_PT)
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  if (pt->page.pt == NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_MAP_FIRST);

  __arch_map_page (pt->pt.pd->pd.base,
                   pt->pt.base,
                   __atomik_capslot_get_page_vaddr (pt),
                   __atomik_access_to_page_attr (pt->pt.access & attr));
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

  __atomik_capslot_set_page_vaddr (pt, PT_START (vaddr));

  __arch_map_pagetable (pd->pd.base, /* Remap'd addr */
                        pt->pt.base, /* Remap'd addr */
                        PT_START (vaddr),
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

  __atomik_capslot_set_page_vaddr (page, PAGE_START (vaddr));

  __arch_map_page (pt->pt.base,     /* Remap'd addr */
                   page->page.base, /* Not remapped */
                   PAGE_START (vaddr),
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

  if (pd->object_type != ATOMIK_OBJTYPE_PD)
  {
    *error = ATOMIK_ERROR_INVALID_CAPABILITY;
    goto fail;
  }

  *error = ATOMIK_SUCCESS;

  if ((page_resolved =
      __arch_resolve_page (pd->pd.base, PAGE_START (vaddr), access, error)) ==
          ATOMIK_INVALID_ADDR)
    goto fail;

  return page_resolved | PAGE_OFFSET (vaddr);

fail:
  return ATOMIK_INVALID_ADDR;
}

int
vspace_can_read (const uintptr_t *vspace, const void *vaddr, size_t size)
{
  uintptr_t this;
  uintptr_t last;
  
  this = PAGE_START ((uintptr_t) vaddr);
  last = PAGE_START ((uintptr_t) vaddr + size - 1);

  while (this <= last)
    if (__arch_resolve_page (
          vspace,
          this,
          ATOMIK_PAGEATTR_READABLE,
          NULL) == ATOMIK_INVALID_ADDR)
      return 0;

  return 1;
}


int
capslot_vspace_switch (capslot_t *pd)
{
  error_t exception = ATOMIK_SUCCESS;

  if (pd == NULL)
  {
    curr_vspace = NULL;
    __arch_switch_vspace (NULL);
  }
  else
  {
    if (pd->object_type != ATOMIK_OBJTYPE_PD)
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

    __arch_switch_vspace (pd->pd.base);

    curr_vspace = pd->pd.base;
  }

fail:
  return -exception;
}
