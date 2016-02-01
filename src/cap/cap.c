/*
 *    cap.c: Common capability functions
 *    Copyright (C) 2015  Gonzalo J. Carracedo
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

#include <atomik/atomik.h>
#include <atomik/cap.h>

#define CAPSLOT_LOOKUP_FAILURE(ex, left, guardv, gbits) \
  {                                                     \
    if (info != NULL)                                   \
    {                                                   \
      info->exception  = ex;                            \
      info->bits_left  = left;                          \
      info->guard      = guardv;                        \
      info->guard_bits = gbits;                         \
    }                                                   \
    return NULL;                                        \
  }

void
capslot_clear (capslot_t *cap)
{
  memset (cap, 0, sizeof (capslot_t));
}

void
cnode_init (capslot_t *cnode)
{
  ATOMIK_ASSERT (cnode->object_type == ATOMIK_OBJTYPE_CNODE);
  
  memset (CNODE_BASE (cnode), 0, CNODE_SIZE (cnode));
}

capslot_t *
capslot_lookup (capslot_t *root, cptr_t addr, unsigned char depth, struct caplookup_exception_info *info)
{
  capslot_t *leaf;
  unsigned int bits_resolved;
  
  cptr_t   guard;
  uint32_t entry;
  
  if (root->object_type != ATOMIK_OBJTYPE_CNODE)
    CAPSLOT_LOOKUP_FAILURE (ATOMIK_CAPLOOKUP_EXCEPTION_INVALID_ROOT, 0, 0, 0)
  
  bits_resolved = root->cnode.guard_bits + root->cnode.size_bits;
  entry  = addr  >> (CPTR_BITS - bits_resolved);
  guard  = entry >> root->cnode.size_bits;
  entry &= (1 << root->cnode.size_bits) - 1;
  
  if (root->cnode.guard_bits > depth ||
      root->cnode.guard != guard)
    CAPSLOT_LOOKUP_FAILURE (ATOMIK_CAPLOOKUP_EXCEPTION_GUARD_MISMATCH,
                            depth,
                            root->cnode.guard,
                            root->cnode.guard_bits)
  
  if (bits_resolved > depth)
    CAPSLOT_LOOKUP_FAILURE (ATOMIK_CAPLOOKUP_EXCEPTION_DEPTH_MISMATCH,
                            depth,
                            bits_resolved,
                            0)
  
  leaf = CNODE_BASE (root) + entry;

  depth -= bits_resolved;
  addr <<= bits_resolved;
  
  /* Node found, no more bits to resolve */

  if (depth == 0)
    return leaf;
  
  if (leaf->object_type == ATOMIK_OBJTYPE_NULL)
    CAPSLOT_LOOKUP_FAILURE (ATOMIK_CAPLOOKUP_EXCEPTION_MISSING_CAPABILITY,
                            depth,
                            0,
                            0)
  else if (leaf->object_type != ATOMIK_OBJTYPE_CNODE)
    CAPSLOT_LOOKUP_FAILURE (ATOMIK_CAPLOOKUP_EXCEPTION_DEPTH_MISMATCH,
                            depth,
                            bits_resolved,
                            0)

  return capslot_lookup (leaf, addr, depth, info);
}

/* According to the manual, the retype operation accepts:
 *
 * 1. CPTR to the untyped object
 * 2. Atomik object we are retyping to
 * 3. (For some objects) Size bits.
 * 4. CPTR to the CNode at the root of the destination CSpace
 * 5. CPTR to the destination CNode. Relative to root.
 * 6. Bits to translate to the destination CNode.
 * 7. Offset inside CNode
 * 8. Number of objects to create
 *
 * Arguments 4-7 are used at API level to compute the destination CNode.
*/

/* Preconditions:
 *
 * We have enough memory in destination
 * All slots in destination are free
 */
int
atomik_untyped_retype (
    capslot_t *ut,
    objtype_t type,
    unsigned int size_bits,
    capslot_t *destination,
    unsigned int count)
{
  error_t exception = ATOMIK_SUCCESS;

  size_t ut_size;
  size_t obj_size;
  size_t total_size;
  uintptr_t curr_address;

  unsigned int i = 0;

  if (ut->object_type != ATOMIK_OBJTYPE_UNTYPED)
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  if (ut->mdb_child != NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_REVOKE_FIRST);

  ut_size    = UT_SIZE (ut);
  obj_size   = 1 << size_bits;
  total_size = obj_size * count;

  /* Check whether we can create all these objects */
  if (total_size > ut_size)
    ATOMIK_FAIL (ATOMIK_ERROR_NOT_ENOUGH_MEMORY);

  curr_address = (uintptr_t) UT_BASE (ut);

  /* Written this way to reuse code. Maybe we should have
   * the loop inside the switch in every case to improve
   * speed. The same for MDB pointers.
   */

  for (i = 0; i < count; ++i)
  {
    if (destination[i].object_type != ATOMIK_OBJTYPE_NULL)
      ATOMIK_FAIL (ATOMIK_ERROR_DELETE_FIRST);

    destination[i].object_type = type;

    /* Initialize object accordingly */
    switch (type)
    {
      case ATOMIK_OBJTYPE_UNTYPED:
        destination[i].ut.base = (void *) curr_address;
        destination[i].ut.size_bits = size_bits;
        destination[i].ut.access = ut->ut.access;

        break;

      case ATOMIK_OBJTYPE_CNODE:
        /* CNode is too small */
        if (size_bits < ATOMIK_CAPSLOT_SIZE_BITS)
          ATOMIK_FAIL (ATOMIK_ERROR_RANGE);

        destination[i].cnode.base = (void *) curr_address;
        destination[i].cnode.size_bits = size_bits - ATOMIK_CAPSLOT_SIZE_BITS;
        destination[i].cnode.access = ut->ut.access;

        /* No guard */
        destination[i].cnode.guard_bits = 0;
        destination[i].cnode.guard      = 0;

        /* Initialize memory to zero */
        cnode_init (&destination[i]);

        break;

      default:
        ATOMIK_FAIL (ATOMIK_ERROR_INVALID_ARGUMENT);
    }

    /* Update MDB pointers */
    if (i > 0)
    {
      destination[i].mdb_prev = &destination[i - 1];
      destination[i - 1].mdb_next = &destination[i];
    }
    else
      destination[i].mdb_prev = NULL;

    destination[i].mdb_next = NULL;
    destination[i].mdb_parent = ut;

    curr_address += obj_size;
  }

  /* All set, we can mark the UT object as retyped */
  ut->mdb_child = destination;

  return ATOMIK_SUCCESS;

fail:
  /* Rollback */
  while (--i >= 0)
    capslot_clear (destination + i);


  return -exception;
}
