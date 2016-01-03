/*
 *    int.c: Interrupt handling
 *    Copyright (C) 2016  Gonzalo J. Carracedo
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

#include <string.h>
#include <i386-seg.h>
#include <i386-int.h>

#include <arch.h>
#include <stdio.h>

/* Note: IDT is paged, no BOOT_SYMBOL declaration needed */
struct idt_entry idt_entries[256];
struct idt_ptr   idt_ptr;

void
i386_handle_kernel_interrupt (struct i386_fault_frame *frame)
{
  printf ("panic: exception 0x%x at kernel code address 0x%x:0x%x\n",  frame->intno, frame->cs, frame->eip);
  printf ("  eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n",
         frame->eax, frame->ebx, frame->ecx, frame->edx);
  printf ("  ebp=0x%08x esp=0x%08x esi=0x%08x edi=0x%08x\n",
         frame->ebp, frame->esp, frame->esi, frame->edi);
  printf ("  eflags=0x%08x error=0x%x\n", frame->eflags, frame->error);
  
  __arch_machine_halt ();
}

void
i386_handle_user_interrupt (void *esp)
{
  
}

void
i386_io_wait (void)
{
  __asm__ __volatile__ ("jmp 1f");
  __asm__ __volatile__ ("1:");
  __asm__ __volatile__ ("jmp 1f");
  __asm__ __volatile__ ("1:");
}

void
i386_idt_set_gate  (uint8_t num, void *base, uint16_t sel, uint8_t flags)
{
  idt_entries[num].base_lo = (uintptr_t) base & 0xffff;
  idt_entries[num].base_hi = ((uintptr_t) base >> 16) & 0xffff;

  idt_entries[num].sel     = sel;
  idt_entries[num].always0 = 0;
  idt_entries[num].flags   = flags;
}

void
i386_init_all_gates (void)
{
  idt_ptr.limit = sizeof (struct idt_entry) * 256 - 1;
  idt_ptr.base  = idt_entries;

  memset (&idt_entries, 0, sizeof (struct idt_entry) * 256);

  /* In Atomik, all gates are interrupt gates to ensure
     that we enter in kernel context with interrupts
     disabled. */
  i386_idt_set_gate  ( 0, isr0 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  ( 1, isr1 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  ( 2, isr2 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  ( 3, isr3 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3)); /* To make int3 work */

  i386_idt_set_gate  ( 4, isr4 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  ( 5, isr5 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  ( 6, isr6 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  ( 7, isr7 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  ( 8, isr8 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  ( 9, isr9 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (10, isr10, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (11, isr11, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (12, isr12, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (13, isr13, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (14, isr14, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (15, isr15, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (16, isr16, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (17, isr17, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (18, isr18, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (19, isr19, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (20, isr20, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (21, isr21, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (22, isr22, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (23, isr23, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (24, isr24, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (25, isr25, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (26, isr26, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (27, isr27, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (28, isr28, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (29, isr29, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (30, isr30, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (31, isr31, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  /* IRQ mapping */
  i386_idt_set_gate  (32, isr32, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (33, isr33, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (34, isr34, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (35, isr35, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (36, isr36, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (37, isr37, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (38, isr38, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (39, isr39, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (40, isr40, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (41, isr41, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (42, isr42, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (43, isr43, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (44, isr44, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (45, isr45, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (46, isr46, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (47, isr47, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  /* IOAPIC extra IRQs */
  
  i386_idt_set_gate  (48, isr48, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (49, isr49, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (50, isr50, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (51, isr51, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (52, isr52, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (53, isr53, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (54, isr54, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  i386_idt_set_gate  (55, isr55, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  /* User-defined interrupt gates */
  i386_idt_set_gate  (56, isr56, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (57, isr57, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (58, isr58, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (59, isr59, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (60, isr60, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (61, isr61, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (62, isr62, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (63, isr63, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (64, isr64, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (65, isr65, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (66, isr66, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (67, isr67, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (68, isr68, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (69, isr69, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (70, isr70, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (71, isr71, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (72, isr72, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (73, isr73, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (74, isr74, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (75, isr75, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (76, isr76, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (77, isr77, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (78, isr78, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (79, isr79, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (80, isr80, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (81, isr81, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (82, isr82, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (83, isr83, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (84, isr84, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (85, isr85, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (86, isr86, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (87, isr87, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (88, isr88, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (89, isr89, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (90, isr90, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (91, isr91, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (92, isr92, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (93, isr93, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (94, isr94, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (95, isr95, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (96, isr96, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (97, isr97, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (98, isr98, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (99, isr99, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (100, isr100, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (101, isr101, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (102, isr102, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (103, isr103, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (104, isr104, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (105, isr105, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (106, isr106, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (107, isr107, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (108, isr108, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (109, isr109, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (110, isr110, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (111, isr111, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (112, isr112, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (113, isr113, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (114, isr114, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (115, isr115, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (116, isr116, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (117, isr117, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (118, isr118, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (119, isr119, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (120, isr120, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (121, isr121, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (122, isr122, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (123, isr123, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (124, isr124, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (125, isr125, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (126, isr126, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (127, isr127, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (128, isr128, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (129, isr129, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (130, isr130, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (131, isr131, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (132, isr132, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (133, isr133, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (134, isr134, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (135, isr135, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (136, isr136, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (137, isr137, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (138, isr138, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (139, isr139, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (140, isr140, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (141, isr141, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (142, isr142, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (143, isr143, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (144, isr144, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (145, isr145, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (146, isr146, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (147, isr147, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (148, isr148, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (149, isr149, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (150, isr150, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (151, isr151, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (152, isr152, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (153, isr153, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (154, isr154, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (155, isr155, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (156, isr156, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (157, isr157, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (158, isr158, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  i386_idt_set_gate  (159, isr159, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));


  /* System calls. */
  i386_idt_set_gate  (160, isr160, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3));

  /* Bugcheck */
  i386_idt_set_gate  (255, isr255, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);
    
  i386_idt_flush (&idt_ptr);
}
