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

int
atomik_capslot_delete (capslot_t *slot)
{
  capslot_t *parent;
  error_t exception = ATOMIK_SUCCESS;
  /* Cannot delete if capability has children */

  if (slot->mdb_child != NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_REVOKE_FIRST);

  if ((parent = slot->mdb_parent) == NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_ILLEGAL_OPERATION);

  if (slot->mdb_prev == NULL)
    parent->mdb_child = slot->mdb_next;
  else
    slot->mdb_prev->mdb_next = slot->mdb_next;

  if (slot->mdb_next != NULL)
    slot->mdb_next->mdb_prev = slot->mdb_prev;

  /* TODO: FOR SECURITY: PERFORM MEMORY CLEANUP OF
   * BIG OBJECTS. */

  /* TODO: Why not perform cleanup in user level? */

  capslot_clear (slot);

fail:
  return -exception;
}

int
atomik_capslot_revoke (capslot_t *slot)
{
  capslot_t *this, *next;
  int error;

  this = slot->mdb_child;

  while (this != NULL)
  {
    next = this->mdb_next;

    if ((error = atomik_capslot_revoke (this)) != ATOMIK_SUCCESS)
      return error;

    if ((error = atomik_capslot_delete (this)) != ATOMIK_SUCCESS)
      return error;

    this = next;
  }

  return ATOMIK_SUCCESS;
}
