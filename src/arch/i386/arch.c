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
#include <atomik/tcb.h>

#include <arch.h>

#include <i386-serial.h>
#include <i386-int.h>
#include <i386-irq.h>
#include <i386-seg.h>
#include <i386-regs.h>
#include <i386-tcb.h>
#include <i386-page.h>
#include <i386-layout.h>
#include <i386-timer.h>

extern void  *free_start;
extern size_t free_size;

extern void  *remap_start;
extern size_t remap_size;

extern void  *kernel_virt_start;
extern void  *kernel_phys_start;
extern size_t kernel_size;

/* This is initialized in boot.c. Points to a page directory */
extern struct page_table *page_dir;


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
  __asm__ __volatile__ ( "invlpg (%0)" : : "r" (addr) : "memory");
}

void
__arch_map_page (void *pt, void *frame, uintptr_t vaddr, uint8_t attr)
{
  uint32_t *x86_pt   = (uint32_t *) pt;
  uintptr_t x86_phys = (uintptr_t) frame;
  uint8_t   x86_attr = 0;

  if (vaddr < KERNEL_BASE ||
      ((vaddr >= KERNEL_VREMAP_BASE) &&
       (vaddr < KERNEL_VREMAP_BASE + KERNEL_VREMAP_MAX)))
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
  uint16_t  x86_attr = 0;
  unsigned int i;
  
  if (x86_pd == NULL)
    x86_pd = (uint32_t *) __atomik_phys_to_remap ((uintptr_t) page_dir);
  
  if (vaddr < KERNEL_BASE ||
      ((vaddr >= KERNEL_VREMAP_BASE) &&
       (vaddr < KERNEL_VREMAP_BASE + KERNEL_VREMAP_MAX)))
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

static inline int
check_page_access (uint16_t x86_control, uint8_t access)
{
  uint16_t mask;

  mask = PAGE_FLAG_PRESENT;

  if (!(access & ATOMIK_PAGEATTR_KERNEL))
    mask |= PAGE_FLAG_USERLAND;
  
  if (access & ATOMIK_PAGEATTR_WRITABLE)
    mask |= PAGE_FLAG_WRITABLE;

  return (x86_control & mask) == mask;
}

/* This returns a remapped address */
uintptr_t *
__arch_resolve_pagetable (const void *pd, uintptr_t vaddr, uint8_t access, error_t *err)
{
  error_t   exception = ATOMIK_SUCCESS;
  uint32_t *x86_pd = (uint32_t *) pd;
  uint32_t  x86_pde;
  uint16_t  x86_attr = 0;

  x86_pde  = x86_pd[VADDR_GET_PDE_INDEX (vaddr)];
  x86_attr = x86_pde & PAGE_CONTROL_MASK;

  if (!check_page_access (x86_attr, access))
    ATOMIK_FAIL (ATOMIK_ERROR_ACCESS_DENIED);

  /* Page table contains physical address, but we need the remapped addres */
  return __atomik_phys_to_remap (x86_pde & PAGE_MASK);

fail:
  if (err != NULL)
    *err = exception;

  return (uintptr_t *) -1;
}

void
__arch_init_tcb (struct tcb *tcb)
{
  tcb->regs.r[I386_TCB_REG_EFLAGS] = EFLAGS_INTERRUPT;
}

/* This always return a physical address */
uintptr_t
__arch_resolve_page (const void *pd, uintptr_t vaddr, uint8_t access, error_t *err)
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

  if (!check_page_access (x86_attr, access))
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
  *virt_start = kernel_virt_start;
  *phys_start = (uintptr_t) kernel_phys_start;

  return kernel_size;
}

/* Necessary to access kernel code from userland. Accepts remapped address */
void
__arch_map_kernel (void *pd)
{
  uint32_t *x86_pd      = (uint32_t *) pd;
  uint32_t *x86_boot_pd =
      (uint32_t *) __atomik_phys_to_remap ((uintptr_t) page_dir);
  size_t    size;
  unsigned int i;
  unsigned int pde_index;

  /* Map kernel */
  size = __UNITS (kernel_size, PTRANGE_SIZE);
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

  /* Map kernel remap */
  size = __UNITS (remap_size, PTRANGE_SIZE);
  for (i = 0; i < size; ++i)
  {
    pde_index = VADDR_GET_PDE_INDEX (
        (uintptr_t) remap_start +
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

/* Initialize TCB registers */
void
__arch_initialize_idle (struct tcb *tcb)
{
  tcb->regs.r[I386_TCB_REG_EIP] = (uintptr_t) &i386_idle_task;
  tcb->regs.r[I386_TCB_REG_EFLAGS] = EFLAGS_INTERRUPT;
}

void
machine_init (void)
{
  i386_serial_init ();
  i386_seg_init ();
  i386_init_all_gates ();
  i386_early_irq_init ();
  i386_setup_timer_freq (HZ);
  i386_timer_enable ();
}

void
enter_multitasking (void)
{
  i386_enter_multitasking ();
}
