/*
 *    irq.c: i386 IRQ initiaization
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
 
#include <stdio.h>

#include <atomik/atomik.h>

#include <i386-irq.h>
#include <i386-regs.h>

void
i386_handle_irq (unsigned int irqno)
{
  outportb (PIC1_COMMAND, PIC_EOI);

  /*
   * These are received by the idle thread. Please note this couldn't
   * happen otherwise as system calls are executed with interrupts
   * disabled.
   */

  /* TODO: Parse IRQ */

  printf ("IRQ #%d\n", irqno);
}

/* io_wait: ensure PIC chip updates its state. This
   is not necessary nowadays (also, we lose several
   cycles in the call and return of this function).*/
static void
io_wait (void)
{
  __asm__ __volatile__ ("jmp 1f");
  __asm__ __volatile__ ("1:");
  __asm__ __volatile__ ("jmp 1f");
  __asm__ __volatile__ ("1:");
}

static void
pic_remap_irq_vector (int offset1, int offset2)
{
  unsigned char a1, a2;

  a1 = inportb (PIC1_DATA);                        // save masks
  a2 = inportb (PIC2_DATA);
  
  outportb (PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait ();
  outportb (PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait ();
  outportb (PIC1_DATA, offset1);
  io_wait ();
  outportb (PIC2_DATA, offset2);
  io_wait ();
  outportb (PIC1_DATA, 4);
  io_wait ();
  outportb (PIC2_DATA, 2);
  io_wait ();
  outportb (PIC1_DATA, ICW4_8086);
  io_wait ();
  outportb (PIC2_DATA, ICW4_8086);
  io_wait ();
  /* Mask all irqs */
  outportb (PIC1_DATA, 0xff);
  outportb (PIC2_DATA, 0xff);
  
  outportb (PIC1_DATA, a1);   // restore saved masks.
  outportb (PIC2_DATA, a2);
}

void
pic_end_of_interrupt (uint8_t irq)
{
  if (irq >= 8)
    outportb (PIC2_COMMAND, PIC_EOI);

  outportb (PIC1_COMMAND, PIC_EOI);
}

static void 
pic_init (void)
{
  pic_remap_irq_vector (PIC_MASTER_INT, PIC_SLAVE_INT); 
}

static void
i386_8259_mask_all (void)
{
  int i;
  
  for (i = 0; i < 16; i++)
    __irq_mask (i);
}

void
i386_early_irq_init (void)
{  
  pic_init ();
  i386_8259_mask_all ();
}

