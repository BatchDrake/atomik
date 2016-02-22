/*
 *    tcb.h: Common TCB definitions
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

#ifndef _ATOMIK_TCB_H
#define _ATOMIK_TCB_H

#include <util.h>
#include <machinedefs.h>
#include <atomik/cap.h>

#define ATOMIK_TCB_SIZE_BITS  7

enum thread_state
{
  ATOMIK_THREAD_STATE_INACTIVE,
  ATOMIK_THREAD_STATE_RUNNING,
  ATOMIK_THREAD_STATE_BLOCKED_ON_SEND,
  ATOMIK_THREAD_STATE_BLOCKED_ON_CALL,
  ATOMIK_THREAD_STATE_BLOCKED_ON_RECV,
  ATOMIK_THREAD_STATE_IDLE_THREAD /* Idle thread state */
};

typedef enum thread_state thread_state_t;

struct tcb
{
  /* Context registers must be at offset 0 */
  tcb_regs_t regs;

  thread_state_t state;
  
  capslot_t *cspace;
  capslot_t *vspace;
  capslot_t *f_ep;

  uint8_t    prio;

  struct tcb *wq_next;
  struct tcb *wq_prev;

  struct tcb *sched_next;
  struct tcb *sched_prev;
};

typedef struct tcb tcb_t;

CPPASSERT (sizeof (tcb_t) < BIT (ATOMIK_TCB_SIZE_BITS));

int atomik_tcb_configure (
                      capslot_t *,
                      capslot_t *,
                      uint8_t,
                      capslot_t *,
                      capslot_t *,
                      capslot_t *);

capslot_t *elf32_load_tcb (void *, size_t, capslot_t *);

#endif /* _ATOMIK_TCB_H */
