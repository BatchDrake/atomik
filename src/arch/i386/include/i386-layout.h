/*
 *    i386-layout.h: Microkernel memory layout in i386
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

#ifndef _ARCH_I386_H
#define _ARCH_I386_H

#define KERNEL_BASE              0xd0000000 /* The kernel starts here */
#define KERNEL_REMAP_MAX         0x20000000 /* 512 MiB of kernel objects max */

#define BOOT_FUNCTION(expr)     expr __attribute__ ((section (".bootcode")))
#define BOOT_SYMBOL(expr)       expr __attribute__ ((section (".bootdata")))

#endif /* _ARCH_I386_H */
