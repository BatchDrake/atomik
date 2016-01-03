/*
 *    arch.c: Architecture-dependent low-level implementations
 *    Copyright (C) 2014  Gonzalo J. Carracedo
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

#include <arch.h>

#include <i386-serial.h>
#include <i386-int.h>
#include <i386-irq.h>
#include <i386-seg.h>

extern void  *free_start;
extern size_t free_size;

extern void  *remap_start;
extern size_t remap_size;

void
__arch_machine_halt (void)
{
  for (;;)
    __asm__ __volatile__ ("hlt");
}

void
__arch_debug_putchar (uint8_t c)
{
  (void) i386_serial_putchar (0, c);
}

void
__arch_get_free_memory (void **pfree_start, size_t *pfree_size)
{
  *pfree_start = free_start;
  *pfree_size  = free_size;
}

void
__arch_get_kernel_remap (void **premap_start, size_t *premap_size)
{
  *premap_start = remap_start;
  *premap_size  = remap_size;
}

void
machine_init (void)
{
  i386_serial_init ();
  i386_seg_init ();
  i386_init_all_gates ();
  i386_early_irq_init ();
}
