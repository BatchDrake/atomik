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

#include <atomik/atomik.h>
#include <atomik/cap.h>
#include <atomik/vspace.h>

#include <arch.h>

#include <i386-serial.h>
#include <i386-int.h>
#include <i386-irq.h>
#include <i386-seg.h>
#include <i386-regs.h>
#include <i386-page.h>
#include <i386-layout.h>

extern void  *free_start;
extern size_t free_size;

extern void  *remap_start;
extern size_t remap_size;

/* This is initialized in boot.c. Points to a page directory */
extern struct page_table *page_dir;

/* Symbols provided by linker */
extern int kernel_start; /* Physical address */
extern int kernel_end;   /* Physical address */
extern int text_start;   /* Virtual address */

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
__arch_invalidate_page (void *addr)
{
  __asm__ __volatile__ ( "invlpg (%0)" : : "b" (addr) : "memory");
}

void
__arch_map_page (void *pt, void *frame, uintptr_t vaddr, uint8_t attr)
{
  uint32_t *x86_pt   = (uint32_t *) pt;
  uintptr_t x86_phys = (uintptr_t) frame;
  uint8_t   x86_attr = 0;

  if (vaddr < KERNEL_BASE)
  {
    if (attr & ATOMIK_PAGEATTR_WRITABLE)
      x86_attr |= PAGE_FLAG_WRITABLE;

    if (attr & ATOMIK_PAGEATTR_PRESENT)
      x86_attr |= PAGE_FLAG_PRESENT;

    if (attr & ATOMIK_PAGEATTR_KERNEL)
      x86_attr |= PAGE_FLAG_GLOBAL;
    else
      x86_attr |= PAGE_FLAG_USERLAND;

    x86_pt[VADDR_GET_PTE_INDEX (vaddr)] = (x86_phys & PAGE_MASK) | x86_attr;
  }
}

void
__arch_map_pagetable (void *pd, void *pt, uintptr_t vaddr, uint8_t attr)
{
  uint32_t *x86_pd   = (uint32_t *) pd;
  uintptr_t x86_pt   = __atomik_remap_to_phys (pt);
  uint8_t   x86_attr = 0;

  if (vaddr < KERNEL_BASE)
  {
    if (attr & ATOMIK_PAGEATTR_WRITABLE)
      x86_attr |= PAGE_FLAG_WRITABLE;

    if (attr & ATOMIK_PAGEATTR_PRESENT)
      x86_attr |= PAGE_FLAG_PRESENT;

    if (attr & ATOMIK_PAGEATTR_KERNEL)
      x86_attr |= PAGE_FLAG_GLOBAL;
    else
      x86_attr |= PAGE_FLAG_USERLAND;

    x86_pd[VADDR_GET_PDE_INDEX (vaddr)] = (x86_pt & PAGE_MASK) | x86_attr;
  }
}

/* This returns a remapped address */
uintptr_t *
__arch_resolve_pagetable (void *pd, uintptr_t vaddr, uint8_t access, error_t *err)
{
  error_t   exception = ATOMIK_SUCCESS;
  uint32_t *x86_pd = (uint32_t *) pd;
  uint32_t  x86_pde;
  uint8_t   x86_attr = 0;

  x86_pde  = x86_pd[VADDR_GET_PDE_INDEX (vaddr)];
  x86_attr = x86_pde & PAGE_CONTROL_MASK;

  if (!(x86_attr & PAGE_FLAG_PRESENT))
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_ADDRESS);

  if ((x86_attr & access) != access)
    ATOMIK_FAIL (ATOMIK_ERROR_ACCESS_DENIED);

  /* Page table contains physical address, but we need the remapped addres */
  return __atomik_phys_to_remap (x86_pde & PAGE_MASK);

fail:
  *err = exception;

  return (uintptr_t *) -1;
}

/* This always return a physical address */
uintptr_t
__arch_resolve_page (void *pd, uintptr_t vaddr, uint8_t access, error_t *err)
{
  error_t   exception = ATOMIK_SUCCESS;
  uint32_t *x86_pt;
  uint32_t  x86_pte;
  uint8_t   x86_attr = 0;

  /* Page table contains physical address, but we need the remapped addres */
  if ((x86_pt = __arch_resolve_pagetable (pd, vaddr, access, &exception)) ==
      (uint32_t *) -1)
    goto fail;

  x86_pte  = x86_pt[VADDR_GET_PTE_INDEX (vaddr)];
  x86_attr = x86_pte & PAGE_CONTROL_MASK;

  if (!(x86_attr & PAGE_FLAG_PRESENT))
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_ADDRESS);

  if ((x86_attr & access) != access)
    ATOMIK_FAIL (ATOMIK_ERROR_ACCESS_DENIED);

  return x86_pte & PAGE_MASK;

fail:
  *err = exception;

  return ATOMIK_INVALID_ADDR;
}

/* Returns size in bytes */
size_t
__arch_get_kernel_layout (void **virt_start, uintptr_t *phys_start)
{
  *virt_start = &text_start;
  *phys_start = (uintptr_t) &kernel_start;

  return (uintptr_t) &kernel_end - (uintptr_t) &kernel_start;
}

/* Necessary to access kernel code from userland. Accepts remapped address */
void
__arch_map_kernel (void *pd)
{
  uint32_t *x86_pd      = (uint32_t *) pd;
  uint32_t *x86_boot_pd =
      (uint32_t *) __atomik_phys_to_remap ((uintptr_t) page_dir);
  size_t    size        =
      __UNITS ((uintptr_t) &kernel_end - (uintptr_t) &kernel_start,
               PTRANGE_SIZE);
  unsigned int i;
  unsigned int pde_index;

  for (i = 0; i < size; ++i)
  {
    pde_index = VADDR_GET_PDE_INDEX (
        KERNEL_BASE +
        (i << (PAGE_BITS + PTE_BITS)));

    x86_pd[pde_index] =
        (x86_boot_pd[pde_index] & PAGE_MASK) |
        PAGE_FLAG_PRESENT |
        PAGE_FLAG_WRITABLE |
        PAGE_FLAG_GLOBAL;
  }
}

/* Switch virtual address space. Accepts remapped address */
void
__arch_switch_vspace (void *pd)
{
  uint32_t *x86_pd;

  if (pd != NULL)
  {
    x86_pd = (uint32_t *) __atomik_remap_to_phys (pd);

    SET_REGISTER ("%cr3", x86_pd);
  }
  else /* Set boot mode page directory */
    SET_REGISTER ("%cr3", page_dir);
}

void
machine_init (void)
{
  i386_serial_init ();
  i386_seg_init ();
  i386_init_all_gates ();
  i386_early_irq_init ();
}
