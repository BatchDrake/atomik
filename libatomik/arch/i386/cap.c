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
