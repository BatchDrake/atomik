/*
 *    util.c: Some useful utility functions
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

#include <alltypes.h>
#include <util.h>

static inline unsigned int
__lsb32 (uint32_t word)
{
  static const int hashpos[] =
    {0,  1, 28,  2,
     29, 14, 24, 3,
     30, 22, 20, 15,
     25, 17, 4, 8,
     31, 27, 13, 23,
     21, 19, 16, 7,
     26, 12, 18, 6,
     11, 5, 10, 9};

  return hashpos[((word & (-word)) * BITTREE_DEBRUIJN32) >> 27];
}

static inline unsigned int
__msb32 (uint32_t word)
{
  unsigned int r = 0;
  static const unsigned int msbs[] =
    {0, 1, 2, 2,
     3, 3, 3, 3,
     4, 4, 4, 4,
     4, 4, 4, 4};

  /* Is there any bit in the upper half? */
  if (0xffff0000 & word)
  {
    r += 16;
    word >>= 16;
  }

  /* Is there any bit in the upper byte? */
  if (0xff00 & word)
  {
    r += 8;
    word >>= 8;
  }

  /* Is there any bit in the upper nibble? */
  if (0xf0 & word)
  {
    r += 4;
    word >>= 4;
  }

  return r + msbs[word];
}

/* The occupation tree using 32 bit nodes implies that:

   Up to 32 blocks: 1 level (1 word).
   Up to 32 * 32 blocks: 2 level (33 words)
   Up to 32 * 32 * 32 blocks: 3 levels (1 + 32 + 1024 words)

   # of levels: MSB / 5 + !!(MSB & 5)
*/

static inline int
bittree_get_levels (unsigned int size)
{
  int msb = __msb32 (size) - 1;

  return msb / 5 + !!(msb % 5);
}

static inline int
bittree_get_bitmap_size (unsigned int levels)
{
  return levels >= 1 ? (1 <<  0) |
        (levels >= 2 ? (1 <<  5) |
        (levels >= 3 ? (1 << 10) |
        (levels >= 4 ? (1 << 15) |
        (levels >= 5 ? (1 << 20) |
        (levels >= 6 ? (1 << 25) |
        (levels >= 7 ? (1 << 30)
        : 0) : 0) : 0) : 0) : 0) : 0) : 0;
}

unsigned int
bittree_find (uint32_t *buf, size_t size)
{
  int levels = bittree_get_levels (size);
  int result = 0;
  uint32_t *node = buf;
  int level = 0;
  int lsb = 0;

  if (*node == BITTREE_FULL_NODE)
    return -1;

  while (level < levels)
  {
    /* Get index of the next node with free objects */
    lsb = __lsb32 (~node[lsb]);

    /* Place index */
    result |= lsb << 5 * (levels - level - 1);

    if (result > size)
      return -1;

    /* Advance in tree: skip all nodes in this level */
    node += 1 << (5 * level++);
  }

  return result;
}

void
bittree_mark (uint32_t *buf, size_t size, unsigned int pos)
{
  uint32_t *node;
  int levels = bittree_get_levels (size);
  int node_ndx, node_bit;
  int offset = bittree_get_bitmap_size (levels - 1);

  /*
   * The trinary operator above behaves as follows:
   *
   * Levels == 1: offset 0    (bitmap in the first word)
   * Levels == 2: offset 1    (bitmap in buf + 1b)
   * Levels == 3: offset 33   (bitmap in buf + 100001b)
   * Levels == 4: offset 1057 (bitmap in buf + 10000100001b)
   *  etc
   */
  do
  {
    node_ndx = pos >> 5;
    node_bit = pos & 0x1f;

    node = &buf[offset + node_ndx];

    *node |= 1 << node_bit;

    /* Next iteration will refer to the position of the parent node */
    pos = node_ndx;

    /* Therefore, the node offset will be reduced */
    offset >>= 5;

    /* Used as exit condition */
    --levels;
  }
  while (levels > 0 && *node == BITTREE_FULL_NODE);
}

void
bittree_unmark (uint32_t *buf, size_t size, unsigned int pos)
{
  uint32_t *node;
  int levels = bittree_get_levels (size);
  int node_ndx, node_bit;
  int offset = bittree_get_bitmap_size (levels - 1);

  /*
   * The trinary operator above behaves as follows:
   *
   * Levels == 1: offset 0    (bitmap in the first word)
   * Levels == 2: offset 1    (bitmap in buf + 1b)
   * Levels == 3: offset 33   (bitmap in buf + 100001b)
   * Levels == 4: offset 1057 (bitmap in buf + 10000100001b)
   *  etc
   */
  do
  {
    node_ndx = pos >> 5;
    node_bit = pos & 0x1f;

    node = &buf[offset + node_ndx];

    *node &= ~(1 << node_bit);

    /* Next iteration will refer to the position of the parent node */
    pos = node_ndx;

    /* Therefore, the node offset will be reduced */
    offset >>= 5;

    /* Used as exit condition */
    --levels;
  }
  while (levels > 0 && *node != BITTREE_FULL_NODE);
}

const char *
error_to_string (error_t err)
{
  switch (err)
  {
    case ATOMIK_SUCCESS:
      return "No errors";

    case ATOMIK_ERROR_INVALID_ARGUMENT:
      return "Invalid argument for function";

    case ATOMIK_ERROR_INVALID_SIZE:
      return "Invalid size for object";

    case ATOMIK_ERROR_INVALID_TYPE:
      return "Invalid type for object";

    case ATOMIK_ERROR_INVALID_CAPABILITY:
      return "Invalid capability type for function";

    case ATOMIK_ERROR_ILLEGAL_OPERATION:
      return "Illegal operation on object";

    case ATOMIK_ERROR_ACCESS_DENIED:
      return "Access denied";

    case ATOMIK_ERROR_INVALID_ADDRESS:
      return "Untranslatable address";

    case ATOMIK_ERROR_RANGE:
      return "Range error";

    case ATOMIK_ERROR_FAILED_LOOKUP:
      return "No such object";

    case ATOMIK_ERROR_DELETE_FIRST:
      return "Object must be deleted first";

    case ATOMIK_ERROR_REVOKE_FIRST:
      return "Derived objects must be deleted first";

    case ATOMIK_ERROR_MAP_FIRST:
      return "Object must be mapped first";

    case ATOMIK_ERROR_UNMAP_FIRST:
      return "Object must be unmapped first";

    case ATOMIK_ERROR_NOT_ENOUGH_MEMORY:
      return "No memory left in object";

    case ATOMIK_ERROR_PAGES_ONLY:
      return "Provided untyped object can only be retyped to page objects";

    case ATOMIK_ERROR_TEST_FAILED:
      return "Microkernel unit test has failed";

    case ATOMIK_ERROR_PULL_FIRST:
      return "Thread already in scheduler queues";

    case ATOMIK_ERROR_PUSH_FIRST:
      return "Thread is not in scheduler queues";

    case ATOMIK_ERROR_ALREADY_BOUND:
      return "Object is already bound to a TCB";

    default:
      return "Unknown error";
  }
}
