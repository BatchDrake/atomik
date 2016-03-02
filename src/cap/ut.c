/*
 *    ut.c: Untyped memory implementation
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
#include <string.h>

#include <atomik/atomik.h>
#include <atomik/cap.h>
#include <atomik/vspace.h>
#include <atomik/tcb.h>

#include <arch.h>

static inline size_t
__atomik_align_watermark (const capslot_t *ut, size_t size_bits)
{
  size_t    size_mask = BIT (size_bits) - 1;
  size_t    watermark = ut->ut.watermark;

  watermark += size_mask;
  watermark &= ~size_mask;

  return watermark;
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
  size_t watermark;
  uintptr_t curr_address;
  int alloc_size_bits;
  int i = 0;

  if (ut->object_type != ATOMIK_OBJTYPE_UNTYPED)
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  if ((exception = __atomik_objtype_adjust_size_bits (type, &size_bits)) !=
      ATOMIK_SUCCESS)
    goto fail;

  watermark  = __atomik_align_watermark (ut, size_bits);
  ut_size    = UT_SIZE (ut);
  obj_size   = BIT (size_bits);
  total_size = obj_size * count;

  if (watermark + total_size > ut_size)
    ATOMIK_FAIL (ATOMIK_ERROR_NOT_ENOUGH_MEMORY);
  
  /* Check whether we are trying to retype a high memory
   * UT into a kernel object. This is not allowed since
   * high memory is only usable by userland processes as
   * page frames.
   */
  if (type != ATOMIK_OBJTYPE_PAGE &&
      type != ATOMIK_OBJTYPE_UNTYPED &&
      type != ATOMIK_OBJTYPE_POOL)
    if (!__atomik_phys_is_remappable (ut->ut.base, total_size))
      ATOMIK_FAIL (ATOMIK_ERROR_PAGES_ONLY);

  curr_address = ((uintptr_t) UT_BASE (ut)) + watermark;

  /* Written this way to reuse code. Maybe we should have
   * the loop inside the switch in every case to improve
   * speed. The same for MDB pointers.
   */

  for (i = 0; i < count; ++i)
  {
    if (destination[i].object_type != ATOMIK_OBJTYPE_NULL)
      ATOMIK_FAIL (ATOMIK_ERROR_DELETE_FIRST);

    if ((exception = capslot_init (
        &destination[i],
        type,
        size_bits,
        ut->ut.access,
        (uintptr_t) ut->ut.base + watermark)) != ATOMIK_SUCCESS)
      goto fail;

    capslot_add_child (ut, &destination[i]);

    curr_address += obj_size;
    watermark    += obj_size;

    ut->ut.watermark = watermark;
  }

  return ATOMIK_SUCCESS;

fail:
  /* Rollback */
  while (--i >= 0)
    capslot_clear (destination + i);


  return -exception;
}
