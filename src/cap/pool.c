/*
 *    pool.c: Object pool capability
 *    Copyright (C) 2016  Gonzalo J. Carracedo
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, oru
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

#include <atomik/atomik.h>
#include <atomik/cap.h>
#include <atomik/vspace.h>


int
atomik_pool_retype (capslot_t *cap, objtype_t type, unsigned int size_bits)
{
  error_t exception = ATOMIK_SUCCESS;
  int levels;
  size_t obj_size;
  size_t pool_objcount;
  size_t bitmap_size;
  size_t bitmap_objcount;

  /*
   * FIXME!!! This will fail miserably if the object pool is not
   * remappable. We need to maintain a kernel-level remap for this
   * kind of things, like a kernel-managed TLB to explore the
   * pool. Also, not al pools are possible.
   */
  uint32_t *root = __atomik_phys_to_remap ((uintptr_t) cap->pool.base);

  unsigned int i;

  if ((exception = __atomik_objtype_adjust_size_bits (type, &size_bits)) !=
      ATOMIK_SUCCESS)
    goto fail;

  if (cap->object_type != ATOMIK_OBJTYPE_POOL)
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  if (cap->pool.pool_type != ATOMIK_OBJTYPE_NULL ||
      type == ATOMIK_OBJTYPE_NULL)
      ATOMIK_FAIL (ATOMIK_ERROR_DELETE_FIRST);

  if (type == ATOMIK_OBJTYPE_POOL || type == ATOMIK_OBJTYPE_UNTYPED)
    ATOMIK_FAIL (ATOMIK_ERROR_ILLEGAL_OPERATION);


  obj_size = BIT (size_bits);
  pool_objcount = cap->pool.size >> size_bits;
  levels = bittree_get_levels (pool_objcount);
  bitmap_size = sizeof (uint32_t) * bittree_get_bitmap_size (levels);
  bitmap_objcount = __UNITS (bitmap_size, obj_size);

  if (type != ATOMIK_OBJTYPE_PAGE)
    if (!__atomik_phys_is_remappable (POOL_BASE (cap), cap->pool.size))
      ATOMIK_FAIL (ATOMIK_ERROR_PAGES_ONLY);

  /* FIXME: DANGEROUS. REMAP WHEN NECESSARY */
  memset (
    __atomik_phys_to_remap ((uintptr_t) POOL_BASE (cap)),
    0,
    bitmap_size);

  /* Mark the first free as sed */
  for (i = 0; i < bitmap_objcount; ++i)
    bittree_mark (root, bitmap_size, i);

  /* Initialize object pool */
  cap->pool.pool_type = type;
  cap->pool.object_size_bits = size_bits;
  cap->pool.size = pool_objcount;
  cap->pool.available = pool_objcount - bitmap_objcount;

fail:
  return -exception;
}

static error_t
pool_alloc (capslot_t *cap, capslot_t *dest, size_t count)
{
  error_t exception = ATOMIK_SUCCESS;
  uint32_t *root;
  void *addr;
  unsigned int index;
  unsigned int i;
  
  if (cap->pool.pool_type == ATOMIK_OBJTYPE_NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_INIT_FIRST);

  if (cap->pool.available < count)
    ATOMIK_FAIL (ATOMIK_ERROR_NOT_ENOUGH_MEMORY);
  
  /*
   * FIXME!!! This will fail miserably if the object pool is not
   * remappable. We need to maintain a kernel-level remap for this
   * kind of things, like a kernel-managed TLB to explore the
   * pool. Also, not al pools are possible.
   */
  root = __atomik_phys_to_remap ((uintptr_t) cap->pool.base);

  for (i = 0; i < count; ++i)
  {
    if (dest[i].object_type != ATOMIK_OBJTYPE_NULL)
      ATOMIK_FAIL (ATOMIK_ERROR_DELETE_FIRST);

    index = bittree_find (root, cap->pool.size);

    ATOMIK_ASSERT (index != -1);

    addr = (index << cap->pool.object_size_bits) + (uint8_t *) cap->pool.base;

    /* Init capslot */
    if ((exception = capslot_init (
           &dest[i],
           cap->pool.pool_type,
           cap->pool.object_size_bits,
           cap->pool.access,
           addr)) != ATOMIK_SUCCESS)
      goto fail;

    /* Update MDB */
    capslot_add_child (cap, &dest[i]);

    /* Mark this object as busy */
    bittree_mark (root, cap->pool.size, index);
    
    /* Decrement object count */
    --cap->pool.available;
  }  
  
fail:
  return exception;
}

/* TODO: add ability to allocate several objects in a row */
int
atomik_pool_alloc (capslot_t *cap, capslot_t *dest, size_t count)
{
  error_t exception = ATOMIK_SUCCESS;

  if (cap->object_type != ATOMIK_OBJTYPE_POOL)
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  exception = pool_alloc (cap, dest, count);
  
fail:
    return -exception;
}

error_t
pool_free (capslot_t *cap, void *buf)
{
  intptr_t objaddr  = (uintptr_t) buf;
  uint32_t *root = (uint32_t *) cap->pool.base;
  size_t mask = BIT (cap->pool.object_size_bits) - 1;
  unsigned int index;

  /* Check alignment */
  if ((objaddr & mask) != 0)
    return ATOMIK_ERROR_INVALID_ARGUMENT;

  index = (objaddr - (intptr_t) root) >> cap->pool.object_size_bits;

  /* Check index */
  if (index < 0 || index >= cap->pool.size)
    return ATOMIK_ERROR_INVALID_ARGUMENT;

  bittree_unmark (root, cap->pool.size, index);

  return ATOMIK_SUCCESS;
}

