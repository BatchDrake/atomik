/*
 *    main.c: the Atomik microkernel entrypoint
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

#include <atomik/atomik.h>
#include <atomik/cap.h>
#include <atomik/tcb.h>

#include <stdio.h>
#include <arch.h>

tcb_t *current;

void
main (void)
{
  int i;

  capslot_t root;
  capslot_t *entry;
  
  machine_init ();

  printf ("Capability size: %d\n", sizeof (capslot_t));

  capabilities_init (&root);

  entry = UT_BASE (&root);

  for (i = 0; i < (1 << root.cnode.size_bits); ++i)
    if (entry[i].object_type == ATOMIK_OBJTYPE_UNTYPED)
      printf ("Entry 0x%02x: %p (%d bits)\n",
              i,
              entry[i].ut.base,
              entry[i].ut.size_bits);

  entry = capslot_lookup (&root, 0xa0100000, 12, NULL);

  printf ("Slot resolved: %p\n", entry);
  
  __arch_machine_halt ();
}
