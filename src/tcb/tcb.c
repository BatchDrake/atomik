/*
 *    tcb.c: Atomik threads implementation
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

#include <stdlib.h>
#include <string.h>

#include <atomik/atomik.h>
#include <atomik/cap.h>
#include <atomik/tcb.h>

tcb_t *curr_tcb; /* If NULL: no task */

int
atomik_tcb_set_regs (capslot_t *tcb, void *regs)
{
  error_t exception = ATOMIK_SUCCESS;

  if (tcb->object_type != ATOMIK_OBJTYPE_TCB)
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  memcpy (&tcb->tcb.base->regs, regs, sizeof (tcb_regs_t));

fail:
  return -exception;
}

static int
__page_belongs_to_pd (capslot_t *page, capslot_t *pd)
{
  if (page->page.pt == NULL)
    return 0;

  if (page->page.pt->pt.pd == NULL)
      return 0;

  /* Comparison is based on the capslot_t pointer because don't want
   * one vanish while the other is still alive. Both are codependent.
   */
  return page->page.pt->pt.pd == pd;
}

#include <stdio.h>
int
atomik_tcb_configure (capslot_t *tcb,
                      capslot_t *f_ep,
                      uint8_t prio,
                      capslot_t *croot,
                      capslot_t *vroot,
                      capslot_t *ipcbuffer)
{
  error_t exception = ATOMIK_SUCCESS;

  /* Ensure we have all the necessary information */
  if (f_ep  == NULL ||
      croot == NULL ||
      vroot == NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_ARGUMENT);

  /* Ensure all objects are of the right type */
  if (tcb->object_type   != ATOMIK_OBJTYPE_TCB      ||
      f_ep->object_type  != ATOMIK_OBJTYPE_ENDPOINT ||
      croot->object_type != ATOMIK_OBJTYPE_CNODE    ||
      vroot->object_type != ATOMIK_OBJTYPE_PD       ||
      (ipcbuffer != NULL &&
          ipcbuffer->object_type != ATOMIK_OBJTYPE_PAGE))
    ATOMIK_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  /* Ensure all objects are not bound yet */
  if (croot->cnode.tcb != NULL || vroot->pd.tcb != NULL)
    ATOMIK_FAIL (ATOMIK_ERROR_ALREADY_BOUND);

  tcb->tcb.base->cspace = croot;

  croot->cnode.tcb = tcb->tcb.base;

  tcb->tcb.base->vspace = vroot;

  if (tcb->tcb.base->cspace != NULL)
    tcb->tcb.base->cspace->cnode.tcb = tcb->tcb.base;

  if (tcb->tcb.base->vspace != NULL)
    tcb->tcb.base->vspace->pd.tcb = tcb->tcb.base;


  tcb->tcb.base->prio = prio;
  tcb->tcb.base->f_ep = f_ep;


fail:
  return -exception;
}

