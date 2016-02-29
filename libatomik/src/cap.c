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

