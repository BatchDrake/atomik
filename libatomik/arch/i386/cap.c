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
#include <stdio.h>
#include <stdlib.h>
#include <system/cap.h>

/* This system call clobbers a lot of registers */
int
__cap_get_info (cptr_t cptr, uint8_t depth, struct capinfo *info)
{
  int __r;
  uint32_t accgrdsiz;

  if (info == NULL) /* Capability check mode */
    __asm__ __volatile__ (
      "int $0xa0"
      : "=a" (__r)
      : "a" (__ASC_cap_get_info),
        "b" (cptr),
        "c" (depth)
      : "edx", "esi", "edi");
  else
  {
    __asm__ __volatile__ (
      "int $0xa0" 
      : "=a" (__r),
        "=b" (info->ci_bits),
        "=c" (accgrdsiz),
        "=d" (info->ci_guard),
        "=S" (info->ci_paddr),
        "=D" (info->ci_vaddr)
      : "a" (__ASC_cap_get_info),
        "b" (cptr),
        "c" (depth));

    info->ci_access     = accgrdsiz;
    info->ci_guard_bits = accgrdsiz >> 8;
    info->ci_type       = __r;
  }

  return __r;
}

int
__cap_delete (cptr_t cptr)
{
  int __r;

  __asm__ __volatile__ (
    "int $0xa0"
    : "=a" (__r)
    : "a" (__ASC_cap_delete),
      "b" (cptr));

  return __r;
}

int
__cap_revoke (cptr_t cptr)
{
  int __r;

  __asm__ __volatile__ (
    "int $0xa0"
    : "=a" (__r)
    : "a" (__ASC_cap_revoke),
      "b" (cptr));

  return __r;
}

int
__cap_drop (cptr_t cptr)
{
  int __r;

  __asm__ __volatile__ (
    "int $0xa0"
    : "=a" (__r)
    : "a" (__ASC_cap_drop),
      "b" (cptr));

  return __r;
}

int
__ut_retype (
  cptr_t ut,
  objtype_t type,
  uintptr_t size_bits,
  uintptr_t depth,
  cptr_t dest,
  uintptr_t offset,
  unsigned int count)
{
  int __r;
  uint32_t ecx;

  ecx = type | (size_bits << 8) | (depth << 16);
  
  __asm__ __volatile__ (
    "int $0xa0" 
    : "=a" (__r)
    : "a" (__ASC_ut_retype),
      "b" (ut),
      "c" (ecx),
      "d" (dest),
      "S" (offset),
      "D" (count));

  return __r;
}

int
__pool_retype (cptr_t pool, objtype_t type, unsigned int size)
{
  int __r;

  __asm__ __volatile__ (
    "int $0xa0" 
    : "=a" (__r)
    : "a" (__ASC_pool_retype),
      "b" (pool),
      "c" (type),
      "d" (size));

  return __r;
}

int
__pool_alloc (cptr_t pool, cptr_t cnode, unsigned int depth, size_t count)
{
  int __r;

  __asm__ __volatile__ (
    "int $0xa0" 
    : "=a" (__r)
    : "a" (__ASC_pool_alloc),
      "b" (pool),
      "c" (cnode),
      "d" (depth),
      "S" (count));

  return __r;
}

int
__pool_free (cptr_t pool, void *buf)
{
  int __r;

  __asm__ __volatile__ (
    "int $0xa0" 
    : "=a" (__r)
    : "a" (__ASC_pool_free),
      "b" (pool),
      "c" (buf));

  return __r;
}
