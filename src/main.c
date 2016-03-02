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
#include <atomik/test.h>
#include <atomik/tcb.h>

#include <stdio.h>
#include <arch.h>

void
main (void *root_task_base, size_t root_task_size)
{
  int i;

  int retcode;
  capslot_t root;
  capslot_t *root_task;

  machine_init ();
  
  capabilities_init (&root);

  if (run_tests (&root) != ATOMIK_SUCCESS)
    __arch_machine_halt ();

  if (root_task_base == NULL)
  {
    printf ("atomik: no root task provided, microkernel halted\n");
    __arch_machine_halt ();
  }
  else if ((root_task = elf32_load_tcb (
                            root_task_base,
                            root_task_size,
                            &root)) == NULL)
  {
    printf ("atomik: invalid root task\n");
    __arch_machine_halt ();
  }

  printf ("atomik: everything seems fine, TCB at %p\n", root_task);

  sched_init ();

  if ((retcode = atomik_sched_push_tcb (root_task)) < 0)
    printf ("atomik: cannot push TCB: %s\n", error_to_string (-retcode));

  enter_multitasking ();
}
