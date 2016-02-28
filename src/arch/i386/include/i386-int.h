/*
 *    i386-int.h: i386 definitions for interrupt handling
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

#ifndef _ARCH_I386_INT_H
#define _ARCH_I386_INT_H

#include <alltypes.h>
#include <util.h>
#include <i386-irq.h>

#define I386_INT_DIVIDE_BY_ZERO               0
#define I386_INT_DEBUG                        1
#define I386_INT_NMI                          2
#define I386_INT_BREAKPOINT                   3
#define I386_INT_OVERFLOW                     4
#define I386_INT_BOUND_RANGE_EXCEEDED         5
#define I386_INT_INVALID_OPCODE               6
#define I386_INT_DEVICE_NOT_AVAILABLE         7
#define I386_INT_DOUBLE_FAULT                 8
#define I386_INT_COPROCESSOR_SEGMENT_OVERRUN  9
#define I386_INT_INVALID_TSS                  10
#define I386_INT_SEGMENT_NOT_PRESENT          11
#define I386_INT_STACK_SEGMENT_FAULT          12
#define I386_INT_GENERAL_PROTECTION_FAULT     13
#define I386_INT_PAGE_FAULT                   14
#define I386_INT_15                           15
#define I386_INT_X87_FLOATING_POINT_EXCEPTION 16
#define I386_INT_ALIGNMENT_CHECK              17
#define I386_INT_MACHINE_CHECK                18
#define I386_INT_SIMD_FPE                     19
#define I386_INT_SECURITY_EXCEPTION           30

#define I386_INTERRUPT_GATE                  0xE /* Disables interrupts */
#define I386_TRAP_GATE                       0xF  

#define IDT_SYSTEM                           0x10
#define IDT_PRIV(r)                          ((r) << 5)
#define IDT_PRESENT                          0x80

#define I386_INT_SYSCALL                     0xa0
#define I386_INT_BUGCHECK                    0xff

#define I386_IRQ_REMAP_START                 PIC_MASTER_INT
#define I386_IRQ_REMAP_END                   (PIC_MASTER_INT + 16)

#define RAISE_INTERRUPT(x) __asm__ __volatile__ ("int $" STRINGIFY (x));

struct idt_entry
{
  uint16_t  base_lo;             
  uint16_t  sel;                 
  uint8_t   always0;             
  uint8_t   flags;               
  uint16_t  base_hi;             
} PACKED;

struct idt_ptr
{
  uint16_t   limit;
  struct idt_entry *base;                
} PACKED;

struct i386_fault_frame
{
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;

  uint32_t intno;
  uint32_t error;

  uint32_t eip;  /* Kernel fault address */
  uint32_t cs;   /* Code segment (in kernel, this should be 0x8) */

  uint32_t eflags; /* Processor flags at the time of fault */
};

void i386_idt_flush (struct idt_ptr *);
void i386_init_all_gates (void);
void i386_idle_task (void);

/* i386 interrupt handlers */
void isr0  (void);
void isr1  (void);
void isr2  (void);
void isr3  (void);
void isr4  (void);
void isr5  (void);
void isr6  (void);
void isr7  (void);
void isr8  (void);
void isr9  (void);
void isr10 (void);
void isr11 (void);
void isr12 (void);
void isr13 (void);
void isr14 (void);
void isr15 (void);
void isr16 (void);
void isr17 (void);
void isr18 (void);
void isr19 (void);
void isr20 (void);
void isr21 (void);
void isr22 (void);
void isr23 (void);
void isr24 (void);
void isr25 (void);
void isr26 (void);
void isr27 (void);
void isr28 (void);
void isr29 (void);
void isr30 (void);
void isr31 (void);

void isr32 (void);
void isr33 (void);
void isr34 (void);
void isr35 (void);
void isr36 (void);
void isr37 (void);
void isr38 (void);
void isr39 (void);
void isr40 (void);
void isr41 (void);
void isr42 (void);
void isr43 (void);
void isr44 (void);
void isr45 (void);
void isr46 (void);
void isr47 (void); 

void isr48 (void); 
void isr49 (void); 
void isr50 (void); 
void isr51 (void); 
void isr52 (void); 
void isr53 (void); 
void isr54 (void); 
void isr55 (void); 

/* User-defined gates */
void isr56 (void);
void isr57 (void);
void isr58 (void);
void isr59 (void);
void isr60 (void);
void isr61 (void);
void isr62 (void);
void isr63 (void);
void isr64 (void);
void isr65 (void);
void isr66 (void);
void isr67 (void);
void isr68 (void);
void isr69 (void);
void isr70 (void);
void isr71 (void);
void isr72 (void);
void isr73 (void);
void isr74 (void);
void isr75 (void);
void isr76 (void);
void isr77 (void);
void isr78 (void);
void isr79 (void);
void isr80 (void);
void isr81 (void);
void isr82 (void);
void isr83 (void);
void isr84 (void);
void isr85 (void);
void isr86 (void);
void isr87 (void);
void isr88 (void);
void isr89 (void);
void isr90 (void);
void isr91 (void);
void isr92 (void);
void isr93 (void);
void isr94 (void);
void isr95 (void);
void isr96 (void);
void isr97 (void);
void isr98 (void);
void isr99 (void);
void isr100 (void);
void isr101 (void);
void isr102 (void);
void isr103 (void);
void isr104 (void);
void isr105 (void);
void isr106 (void);
void isr107 (void);
void isr108 (void);
void isr109 (void);
void isr110 (void);
void isr111 (void);
void isr112 (void);
void isr113 (void);
void isr114 (void);
void isr115 (void);
void isr116 (void);
void isr117 (void);
void isr118 (void);
void isr119 (void);
void isr120 (void);
void isr121 (void);
void isr122 (void);
void isr123 (void);
void isr124 (void);
void isr125 (void);
void isr126 (void);
void isr127 (void);
void isr128 (void);
void isr129 (void);
void isr130 (void);
void isr131 (void);
void isr132 (void);
void isr133 (void);
void isr134 (void);
void isr135 (void);
void isr136 (void);
void isr137 (void);
void isr138 (void);
void isr139 (void);
void isr140 (void);
void isr141 (void);
void isr142 (void);
void isr143 (void);
void isr144 (void);
void isr145 (void);
void isr146 (void);
void isr147 (void);
void isr148 (void);
void isr149 (void);
void isr150 (void);
void isr151 (void);
void isr152 (void);
void isr153 (void);
void isr154 (void);
void isr155 (void);
void isr156 (void);
void isr157 (void);
void isr158 (void);
void isr159 (void);

/* Microkernel syscall */
void isr160 (void);

/* Bugcheck interrupt */
void isr255 (void); 

/* Handle syscall */
void i386_handle_syscall (unsigned int);

#endif /* _ARCH_I386_INT_H */
