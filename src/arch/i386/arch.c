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
machine_init (void)
{
  i386_serial_init ();
}
