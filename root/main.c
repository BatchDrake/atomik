/*
 *    main.c: Entry point for the root task
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
#include <system/cap.h>

void
_start (void)
{
  unsigned int i;
  struct capinfo info;
  uint32_t cptr, guard;
  char guard_bits, bits;
  unsigned int entries;
  
  printf ("Hello world (_start @ %p)\n", _start);

  if (cap_get_info (0, 0, &info) == -1)
    printf ("root: cspace lookup failed\n");
  else
  {
    entries = 1 << info.ci_bits; 
    printf (
      "CSpace found: type %d, %d entries\n",
      info.ci_type,
      entries);

    printf (
      "CSpace found: guard is 0x%x (%d bits)\n",
      info.ci_guard,
      info.ci_guard_bits);

    bits = info.ci_bits;
    guard = info.ci_guard;
    guard_bits = info.ci_guard_bits;

    printf ("CSpace contents:\n");
    
    for (i = 0; i < entries; ++i)
    {
      cptr = guard << (32 - guard_bits);
      cptr |= i << (32 - guard_bits - bits);

      if (cap_get_info (cptr, guard_bits + bits, &info) != -1) 
        printf (
          "[%08x] Object of type %d (%d bytes / entries)\n",
          cptr,
          info.ci_type,
          1 << info.ci_bits);
    }
  }
  
  d_halt ();
}
