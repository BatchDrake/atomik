/*
 *    sched.c: Round-robin scheduler
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
#include <string.h>
#include <stdlib.h>

#include <atomik/atomik.h>
#include <atomik/cap.h>
#include <atomik/tcb.h>

struct sched_queue
{
  struct sched_queue *prev; /* Queue of bigger priority */
  struct sched_queue *next; /* Queue of smaller priority */

  tcb_t *head;
  tcb_t *tail;
};

tcb_t idle_tcb;
tcb_t *curr_tcb;

struct sched_queue *curr_queue;
struct sched_queue sched_queues[ATOMIK_SCHED_QUEUE_COUNT];

static uint8_t  min_prio;
static uint8_t  max_prio;

static inline int
sched_is_idle (void)
{
  return curr_tcb == &idle_tcb;
}

static inline int
sched_queue_is_idle (uint8_t n)
{
  struct sched_queue *q = &sched_queues[n];

  return q->head == NULL;
}


static inline void
sched_queue_set_as_idle (uint8_t n)
{
  struct sched_queue *q = &sched_queues[n];

  if (q->prev != NULL)
    q->prev->next = q->next;

  if (q->next != NULL)
    q->next->prev = q->prev;
}

static inline void
sched_queue_set_as_running (uint8_t n)
{
  struct sched_queue *q = &sched_queues[n];
  int i;

  q->prev = NULL;
  q->next = NULL;

  for (i = n - 1; i >= min_prio; --i)
    if (!sched_queue_is_idle (i))
    {
      q->prev = &sched_queues[i];
      q->prev->next = q;
      break;
    }

  for (i = n + 1; i <= max_prio; ++i)
    if (!sched_queue_is_idle (i))
    {
      q->next = &sched_queues[i];
      q->next->prev = q;
      break;
    }

  if (q->prev == NULL)
    min_prio = n;

  if (q->next == NULL)
    min_prio = n;
}

static inline void
sched_queue_push_tcb (uint8_t n, tcb_t *tcb)
{
  struct sched_queue *q = &sched_queues[n];

  if (sched_queue_is_idle (n))
  {
    q->head = q->tail = tcb;

    sched_queue_set_as_running (n);

    /* If we have no higher-priority queues above us we set ourselves as the
     * current running queue.
     */

    if (q->prev == NULL)
    {
      /* Schedule automatically */
      curr_queue = q;
      curr_tcb   = tcb;
    }
  }

  tcb->sched_prev = q->tail;
  tcb->sched_next = q->head;

  q->tail->sched_next = tcb;
  q->head->sched_prev = tcb;

  q->tail = tcb;
}

static inline void
sched_queue_pull_tcb (uint8_t n, tcb_t *tcb)
{
  struct sched_queue *q = &sched_queues[n];

  /* Is the last thread in the queue? */
  if (q->head == tcb && q->tail == tcb)
  {
    q->head = q->tail = NULL;
    tcb->sched_prev = tcb->sched_next = NULL;

    sched_queue_set_as_idle (n);

    /* Reschedule if necessary */
    if (curr_queue == q)
    {
      if ((curr_queue = curr_queue->next) == NULL) /* Move to next queue */
        curr_tcb = &idle_tcb; /* Okay, we ran out of threads */
      else
        curr_tcb = curr_queue->head;
    }
  }
  else
  {
    /* Is the current running thread? */
    if (tcb == curr_tcb)
      atomik_sched_schedule ();

    if (q->head == tcb)
      q->head = tcb->sched_next;

    if (q->tail == tcb)
        q->tail = tcb->sched_prev;

    tcb->sched_prev->sched_next = tcb->sched_next;
    tcb->sched_next->sched_prev = tcb->sched_prev;
  }
}


int
atomik_sched_push_tcb (capslot_t *cap)
{
  error_t exception = ATOMIK_SUCCESS;
  tcb_t *tcb;

  if (cap->object_type != ATOMIK_OBJTYPE_TCB)
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  tcb = cap->tcb.base;

  if (tcb->sched_next != NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_PULL_FIRST);

  sched_queue_push_tcb (tcb->prio, tcb);

fail:
  return -exception;
}

int
atomik_sched_pull_tcb (capslot_t *cap)
{
  error_t exception = ATOMIK_SUCCESS;
  tcb_t *tcb;

  if (cap->object_type != ATOMIK_OBJTYPE_TCB)
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  tcb = cap->tcb.base;

  if (tcb->sched_next != NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_PUSH_FIRST);

  sched_queue_pull_tcb (tcb->prio, tcb);

  fail:
    return -exception;
}

void
atomik_sched_schedule (void)
{
  if (!sched_is_idle ())
    curr_tcb = curr_tcb->sched_next;
}

void
sched_init (void)
{
  curr_tcb = &idle_tcb;
}
