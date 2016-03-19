/*
 *    machinedefs.h: Architecture-specific parameters needed by the
 *    machine-independent code of the microkernel.
 *
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

#ifndef _ARCH_MACHINEDEFS_H
#define _ARCH_MACHINEDEFS_H

#include <i386-tcb.h>
#include <i386-layout.h>

#define HZ 100

#define VIRT_ADDR_ORDER 2
#define PHYS_ADDR_BITS  BIT (VIRT_ADDR_ORDER + 3)
#define VIRT_ADDR_BITS  BIT (VIRT_ADDR_ORDER + 3)

#define PAGE_BITS      12
#define PTE_BITS       10
#define PDE_BITS       10

#define PT_BITS        (PTE_BITS + VIRT_ADDR_ORDER)
#define PD_BITS        (PDE_BITS + VIRT_ADDR_ORDER)

#define PREFERED_STACK_SIZE BIT (PAGE_BITS + 4)
#define PREFERED_STACK_BASE (KERNEL_BASE - PREFERED_STACK_SIZE)

#endif /* _ARCH_MACHINEDEFS_H */
