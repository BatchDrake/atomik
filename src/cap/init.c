/*
 *    init.c: Configuration of initial set of capabilities
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
#include <stdlib.h>
#include <atomik/cap.h>
#include <arch.h>

#include <cap_init.h>

void
capabilities_init (capslot_t *root)
{
  capslot_t *cap;
  void  *free_start;
  size_t free_size;

  void  *remap_start;
  size_t remap_size;

  uintptr_t curr_block;
  
  unsigned int i, n;
  
  __arch_get_free_memory (&free_start, &free_size);
  __arch_get_kernel_remap (&remap_start, &remap_size);

  printf ("Free  memory: %p (%d bytes)\n", free_start,  free_size);
  printf ("Remap memory: %p (%d bytes)\n", remap_start, remap_size);
  
  cap        = (capslot_t *) remap_start;
  curr_block = (uintptr_t)   free_start + PAGE_SIZE;

  /* In the worst case scenario (64 bits, fully unaligned memory)
     we would require up to 2 * (64 - 12) = 104 untyped memory
     capabilities = 3328 bytes that fit in a single 4KiB page.
  */

  root->object_type      = ATOMIK_OBJTYPE_CNODE;
  root->cnode.size_bits  = ATOMIK_INIT_CAP_CNODE_SIZE;
  root->cnode.guard_bits = ATOMIK_INIT_CAP_GUARD_BITS;
  root->cnode.guard      = ATOMIK_INIT_CAP_GUARD;
  root->cnode.base       = cap;

  cnode_init (root);
  
  n = 0;

  while (free_size > 0)
  {
    for (i = 0; i < VIRT_ADDR_BITS; ++i)
      if ((curr_block & (1 << i)) ||
          (1 << (i + 1)) > free_size)
        break;

    cap[n].object_type  = ATOMIK_OBJTYPE_UNTYPED;
    cap[n].ut.size_bits = i;
    
    cap[n].cnode.base = curr_block;

    ++n;
    curr_block += 1 << i;
    free_size  -= 1 << i;
  }
}
