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

/* How to implement the buddy allocator:
 *
 * 1. Create a struct pool_state with all the temporary
 *    values required to deal with a pool and work on top
 *    of this pool_state.
 *
 * 2. Allocate blocks for all objects.
 * 3. Remove blocks from metadata.
 *
 *
 */
struct buddyhdr
{
  struct buddyhdr *next;
  struct buddyhdr *prev;
};

struct pool_state
{
  void    *base;              /* P */
  void    *blocks;            /* P */
  uint8_t *bitmap;            /* V */
  struct buddyhdr **freelist; /* V */

  uint8_t levels;
  uint8_t object_size_bits;
  size_t  object_size;

  size_t size;
  size_t available;
};

static inline uint8_t
__get_bits (size_t size)
{
  uint8_t bits;

  bits = __msb32 (size) - 1;

  if (BIT (bits) == size)
    ++bits;

  return bits;
}

static inline uint8_t
__poolcap_get_levels (const capslot_t *cap)
{
  return __get_bits (cap->pool.size);
}

static inline size_t
__poolcap_get_bitmap_size (const capslot_t *cap)
{
  unsigned int levels = __poolcap_get_levels (cap);

  return BIT (levels < 3 ? 0 : levels - 3);
}

static inline size_t
__poolcap_get_metadata_size (const capslot_t *cap)
{
  unsigned int levels = __poolcap_get_levels (cap);

  return
      sizeof (size_t) +
      levels * sizeof (void *) +
      __poolcap_get_bitmap_size (cap);
}

static inline void
__pool_state_init (struct pool_state *state, const capslot_t *cap)
{
  size_t *meta_size  = __atomik_phys_to_remap ((uintptr_t) cap->pool.base);

  state->base        = cap->pool.base;
  state->freelist    = __atomik_phys_to_remap (
                          sizeof (size_t) + (uintptr_t) cap->pool.base);
  state->levels      = __get_bits (cap->pool.size);
  state->bitmap      = (uint8_t *) &state->freelist[state->levels];
  state->object_size_bits = cap->pool.object_size_bits;
  state->object_size = BIT (state->object_size_bits);
  state->size        = cap->pool.size;
  state->available   = cap->pool.available;
  state->blocks      =
      (void *) ((uintptr_t) state->base +
          (*meta_size << state->object_size_bits));
}

/*
 * Let's assume a pool of 8 objects.
 *   L = 4
 *
 * The bitmap has this layout:
 * 0: X        Order: 3
 * 1: XX       Order: 2
 * 3: XXXX     Order: 1
 * 7: XXXXXXXX Order: 0
 *
 * The bitmap for order X is @ BIT (L - X - 1) - 1
 */
static inline void
__pool_toggle_bit (struct pool_state *state, unsigned int index, uint8_t order)
{
  unsigned int bitindex = (BIT (state->levels - order - 1) - 1) +
      (index >> order);
  unsigned int byte     = bitindex >> 3;
  unsigned int bit      = bitindex & 7;

  ATOMIK_ASSERT (index < state->size);

  state->bitmap[byte] ^= BIT (bit);
}

static inline int
__pool_read_bit (
    const struct pool_state *state,
    unsigned int index,
    uint8_t order)
{
  unsigned int bitindex = (BIT (state->levels - order - 1) - 1) +
      (index >> order);
  unsigned int byte     = bitindex >> 3;
  unsigned int bit      = bitindex & 7;

  ATOMIK_ASSERT (index < state->size);

  return state->bitmap[byte] & BIT (bit);
}

static inline unsigned int
__pool_get_object_index (const struct pool_state *state, void *addr /* P */)
{
  return ((uintptr_t) addr - (uintptr_t) state->blocks) >>
      state->object_size_bits;
}

static inline void * /* P */
__pool_get_object_base (const struct pool_state *state, unsigned int index)
{
  return (void *)
      ((uintptr_t) state->blocks + (index << state->object_size_bits));
}

static inline void
__pool_insert_buddy (
    struct pool_state *state,
    struct buddyhdr *new, /* P */
    uint8_t order)
{
  struct buddyhdr *list_head_remap; /* V */
  struct buddyhdr *new_remap;       /* V */

  new_remap = __atomik_phys_to_remap ((uintptr_t) new);

  new_remap->prev = NULL;
  new_remap->next = state->freelist[order];

  if (state->freelist[order] != NULL)
  {
    list_head_remap =
        __atomik_phys_to_remap ((uintptr_t) state->freelist[order]);

    list_head_remap->prev = new;
  }

  state->freelist[order] = new;
}

static inline void
__pool_remove_buddy (
    struct pool_state *state,
    struct buddyhdr *new, /* P */
    uint8_t order)
{
  struct buddyhdr *adj_remap; /* V */
  struct buddyhdr *new_remap; /* V */

  new_remap = __atomik_phys_to_remap ((uintptr_t) new);

  if (new_remap->prev == NULL)
    state->freelist[order] = new_remap->next;
  else
  {
    adj_remap = __atomik_phys_to_remap ((uintptr_t) new_remap->prev);
    adj_remap->next = new_remap->next;
  }

  if (new_remap->next != NULL)
  {
    adj_remap = __atomik_phys_to_remap ((uintptr_t) new_remap->next);
    adj_remap->prev = new_remap->prev;
  }
}

/*
 * Preconditions: big_o must have blocks
 */
static inline void
__pool_split_buddy (struct pool_state *state, uint8_t big_o, uint8_t small_o)
{
  unsigned int index;
  struct buddyhdr *buddy_1, *buddy_2; /* P, P */

  while (big_o > small_o)
  {
    /* In this level, object blocks are of size BIT (big_o)
     * therefore, each buddy is BIT (big_o - 1) apart of the other.
     */
    buddy_1 = state->freelist[big_o];
    index   = __pool_get_object_index (state, buddy_1);
    buddy_2 = __pool_get_object_base  (state, index + BIT (big_o - 1));

    /* Remove block from list */
    __pool_remove_buddy (state, buddy_1, big_o);

    /* Mark block as busy */
    __pool_toggle_bit (state, index, big_o);

    /* Add both buddies as new blocks of lower small_o */
    __pool_insert_buddy (state, buddy_1, big_o - 1);
    __pool_insert_buddy (state, buddy_2, big_o - 1);

    --big_o;
  }
}

static inline struct buddyhdr *
__pool_alloc_buddy (struct pool_state *state, uint8_t order)
{
  unsigned int i;
  unsigned int index;

  struct buddyhdr *buddy_1, *buddy_2; /* P, P */

  for (i = order; i < state->levels; ++i)
    if (state->freelist[i] != NULL)
      break;

  if (i == state->levels)
    return NULL;

  /* Ensure we have blocks in the desired order */
  __pool_split_buddy (state, i, order);

  /* Take first block of this order. This block is ensured to exist
   * by the previous loop
   */
  buddy_1 = state->freelist[order];
  index = __pool_get_object_index (state, buddy_1);

  /* Remove the block from the list */
  __pool_remove_buddy (state, buddy_1, order);

  /* Mark block as used */
  __pool_toggle_bit (state, index, order);

  /* It should be marked as used now */
  ATOMIK_ASSERT (__pool_read_bit (state, index, order));

  return buddy_1;
}

static inline int
__pool_update_parents (struct pool_state *state, unsigned int index, uint8_t order)
{
  int i;

  for (i = order; i < state->levels; ++i)
    if (__pool_read_bit (state, index, i))
      break;

  if (i < state->levels)
  {
    /* Mark all parent buddies as busy */
    while (--i >= order)
    {
      ATOMIK_ASSERT (!__pool_read_bit (state, index, i));

      __pool_toggle_bit (state, index, i);

      ATOMIK_ASSERT (__pool_read_bit (state, index, i));
      __pool_toggle_bit (state, index ^ BIT (i), i);

      /* Check whether they are marked as busy */
      ATOMIK_ASSERT (__pool_read_bit (state, index, i));
      ATOMIK_ASSERT (__pool_read_bit (state, index ^ BIT (i), i));
    }

    return 1;
  }


  return 0;
}

/*
 * The freeing algorithm is based on the following property: if a block
 * is allocated, the upper-level block must also be allocated.
 */

static inline void
__pool_free_buddy (struct pool_state *state, void *addr, uint8_t order)
{
  unsigned int index_1;
  unsigned int index_2;

  void *buddy_1; /* P */
  void *buddy_2; /* P */

  index_1 = __pool_get_object_index (state, addr);
  index_2 = index_1 ^ BIT (order);

  /* Ensure all parent blocks are marked as busy */
  ATOMIK_ASSERT (__pool_update_parents (state, index_1, order));

  /* Before inserting, this block should be busy */
  ATOMIK_ASSERT (__pool_read_bit (state, index_1, order));

  /* Add block */
  __pool_insert_buddy (state, addr, order);

  /* Mark block as free */
  __pool_toggle_bit (state, index_1, order);

  /* Coalesce until no more coalitions are possible */
  while (
      order < state->levels &&
      index_2 + BIT (order) <= state->size &&
      !__pool_read_bit (state, index_2, order))
  {
    buddy_1 = __pool_get_object_base (state, index_1 & ~BIT (order));
    buddy_2 = __pool_get_object_base (state, index_2 | BIT (order));

    /* Remove both buddies */
    __pool_remove_buddy (state, buddy_1, order);
    __pool_remove_buddy (state, buddy_2, order);

    ++order;

    /* Before inserting, parent should be busy */
    ATOMIK_ASSERT (__pool_read_bit (state, index_1, order));

    /* Insert new free block */
    __pool_insert_buddy (state, buddy_1, order);

    /* Mark parent block as free */
    __pool_toggle_bit (state, index_1, order);

    /* Buddy of the parent */
    index_2 = index_1 ^ BIT (order);
  }
}

int
atomik_pool_retype (capslot_t *cap, objtype_t type, unsigned int size_bits)
{
  error_t exception = ATOMIK_SUCCESS;

  size_t obj_size;
  size_t pool_objcount;

  size_t *metadata_objs;

  unsigned int i = 0;

  unsigned int order;
  size_t obj_count;

  struct pool_state state;
  struct buddyhdr *freehdr;

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

  cap->pool.pool_type = type;
  cap->pool.object_size_bits = size_bits;
  cap->pool.size = pool_objcount;

  metadata_objs = __atomik_phys_to_remap ((uintptr_t) cap->pool.base);
  *metadata_objs = __UNITS (__poolcap_get_metadata_size (cap), obj_size);

  cap->pool.available = pool_objcount - *metadata_objs;

  __pool_state_init (&state, cap);

  while (i < pool_objcount)
  {
    /* It's MSB because it has to do with the SIZE of the blocks */
    /* Even though this is not the way to compute the order of a block
     * (we do it in base of the alignment), we ensure that all of
     * them are aligned. The first block will always be aligned.
     * The second block, as it is smaller that the first one
     * has to be forcefully aligned too, as it starts at a bigger
     * power of two, etc.
     */
    order = __msb32 (pool_objcount - i) - 1; /* It will never be zero */
    obj_count = BIT (order);

    __pool_insert_buddy (&state, __pool_get_object_base (&state, i), order);

    ATOMIK_ASSERT (state.freelist[order] != NULL);

    freehdr = __atomik_phys_to_remap ((uintptr_t) state.freelist[order]);

    /* No adjacent blocks */
    freehdr->next = NULL;
    freehdr->prev = NULL;

    i += obj_count;
  }

fail:
  return -exception;
}

static error_t
pool_alloc (capslot_t *cap, capslot_t *dest, size_t count)
{
  error_t exception = ATOMIK_SUCCESS;
  void *addr;

  size_t total = 0;
  unsigned int buddysz;
  int order;
  unsigned int i;

  struct pool_state state;

  if (cap->pool.pool_type == ATOMIK_OBJTYPE_NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_INIT_FIRST);

  if (cap->pool.available < count)
    ATOMIK_FAIL (ATOMIK_ERROR_NOT_ENOUGH_MEMORY);

  __pool_state_init (&state, cap);
  
  /* TODO:
   * 1. Get biggest block <= count
   * 2. If it doesn't exist, split biggest block in two until found.
   * 3. If no biggest blocks are found, move to next size.
   * 4. If exists, allocate and decremento count accordingly
   */

  for (order = __msb32 (count); order >= 0; --order)
  {
    buddysz = BIT (order);

    while (buddysz <= count &&
          (addr = __pool_alloc_buddy (&state, order)) != NULL)
    {
      /* Initialize all objects in the pool */
      for (i = 0; i < buddysz; ++i)
      {
        exception = capslot_init (
            &dest[total + i],
            cap->pool.object_type,
            cap->pool.object_size_bits,
            cap->pool.access,
            (void *) ((uintptr_t) addr + (i << cap->pool.object_size_bits)));

        if (exception != ATOMIK_SUCCESS)
        {
          /* Delete all objects in this block */
          while (--i >= 0)
            atomik_capslot_delete (&dest[total + i]);

          /* Return block to the pool */
          __pool_free_buddy (&state, addr, order);

          goto fail;
        }

        capslot_add_child (cap, &dest[total + i]);

        ATOMIK_ASSERT (dest[total + i].mdb_parent == cap);
      }

      total += buddysz;
      count -= buddysz;
    }

    /* Either buddy is too big to allocate remaining objects, or
     * we ran out of buddies of this size. */
  }

  ATOMIK_ASSERT (count == 0);

fail:
  cap->pool.available -= total;

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
  unsigned int index;
  unsigned int order;

  struct pool_state state;

  __pool_state_init (&state, cap);
  index = __pool_get_object_index (&state, buf);

  if (index > cap->pool.size)
    return ATOMIK_ERROR_INVALID_ADDRESS;

  /* This should be possible in a better way */
  __pool_free_buddy (&state, buf, 0);

  ++cap->pool.available;

  return ATOMIK_SUCCESS;
}

