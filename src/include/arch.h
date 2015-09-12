/*
 *    arch.h: Prototypes for the architecture-dependant low-level interface
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

#ifndef _ARCH_H
#define _ARCH_H

#include <alltypes.h>

/* Send a character to the debug device (usually the serial port) */
void __arch_debug_putchar (uint8_t);

/* Halt machine */
void __arch_machine_halt (void);

 /* Initialize hardware (generic way) */
void machine_init (void);

#endif /* _ARCH_H */
