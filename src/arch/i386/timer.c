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

#include <atomik/atomik.h>

#include <i386-io.h>
#include <i386-irq.h>
#include <i386-timer.h>

void
i386_setup_timer_freq (unsigned int freq)
{
  uint32_t timer_count;

  timer_count = TIMER_BASE_FREQ / freq;

  outportb (TIMER_PORT_COMMAND, TIMER_MODE_3 |
                                TIMER_ACCESS_LOHI |
                                TIMER_CHANNEL (0));

  outportb (TIMER_PORT_CHANNEL (0), timer_count & 0xff);
  outportb (TIMER_PORT_CHANNEL (0), timer_count >> 8);
}

void
i386_timer_enable (void)
{
  __irq_unmask (IRQ_TIMER);
}
