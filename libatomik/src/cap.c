/*
 *    cap.c: Capability system calls
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

#include <system/cap.h>
#include <errno.h>

int
cap_get_info (cptr_t cptr, uint8_t depth, struct capinfo *info)
{
  int __r;

  __r = __cap_get_info (cptr, depth, info);

  if (__r < 0)
  {
    errno = -__r;
    return -1;
  }
  
  return 0;
}

int
cap_delete (cptr_t cptr)
{
  int __r;

  __r = __cap_delete (cptr);

  if (__r < 0)
  {
    errno = -__r;
    return -1;
  }
  
  return 0;
}

int
cap_revoke (cptr_t cptr)
{
  int __r;

  __r = __cap_revoke (cptr);

  if (__r < 0)
  {
    errno = -__r;
    return -1;
  }
  
  return 0;
}

int
cap_drop (cptr_t cptr)
{
  int __r;

  __r = __cap_drop (cptr);

  if (__r < 0)
  {
    errno = -__r;
    return -1;
  }
  
  return 0;
}

int
ut_retype (
  cptr_t ut,
  objtype_t type,
  uint8_t size_bits,
  uint8_t depth,
  cptr_t dest,
  uintptr_t offset,
  unsigned int count)
{
  int __r;

  __r = __ut_retype (
    ut,
    (uint8_t) type,
    size_bits,
    depth,
    dest,
    offset,
    count);

  if (__r < 0)
  {
    errno = -__r;
    return -1;
  }
  
  return 0;
}

int
pool_retype (cptr_t pool, objtype_t type, unsigned int size)
{
  int __r;

  __r = __pool_retype (pool, type, size);
  
  if (__r < 0)
  {
    errno = -__r;
    return -1;
  }
  
  return 0;
}

int
pool_alloc (cptr_t pool, cptr_t cnode, unsigned int depth, size_t count)
{
  int __r;

  __r = __pool_alloc (pool, cnode, depth, count);
  
  if (__r < 0)
  {
    errno = -__r;
    return -1;
  }
  
  return 0;
}

int
pool_free (cptr_t pool, void *buf)
{
  int __r;

  __r = __pool_free (pool, buf);
  
  if (__r < 0)
  {
    errno = -__r;
    return -1;
  }
  
  return 0;
}
