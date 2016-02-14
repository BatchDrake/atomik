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

/* Get free memory layout */
void __arch_get_free_memory (void **, size_t *);

/* Get kernel object remap area */
void __arch_get_kernel_remap (void **, size_t *);

/* Map page to page table */
void __arch_map_page (void *, void *, uintptr_t, uint8_t);

/* Map page table to page directory */
void __arch_map_pagetable (void *, void *, uintptr_t, uint8_t);

/* Resolve existing map */
uintptr_t __arch_resolve_page (void *, uintptr_t, uint8_t, error_t *);

/* Get remapped page table address */
uintptr_t *__arch_resolve_pagetable (void *, uintptr_t, uint8_t, error_t *);

/* Map kernel addresses to page directory */
void __arch_map_kernel (void *);

/* Get kernel image size */
size_t __arch_get_kernel_layout (void **, uintptr_t *);

/* Invalidate page */
void __arch_invalidate_page (void *);

/* Switch vspace */
void __arch_switch_vspace (void *);

/* Initialize hardware (generic way) */
void machine_init (void);

#endif /* _ARCH_H */
