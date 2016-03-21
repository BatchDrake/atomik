/*
 *    syscall.c: Syscall decoding
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

#include <atomik/atomik.h>
#include <atomik/tcb.h>
#include <atomik/vspace.h>

#include <arch.h>

#include <i386-seg.h>
#include <i386-int.h>
#include <i386-regs.h>
#include <i386-irq.h>

#include <atomik-user.h> /* Needed for syscall numbers */

extern tcb_t *curr_tcb;
extern uintptr_t *curr_vspace;

#define ASC_FUNC(name) JOIN (__i386_decode_, name)
#define ASC_PROTO(name) \
  static inline void ASC_FUNC(name) (void)
#define ASC_ID(name)    JOIN (__ASC_, name)
#define ASC_CASE(name)                          \
  case ASC_ID (name):                           \
               ASC_FUNC (name) ();              \
               break

#define SYSCALL_REG(n) curr_tcb->regs.r[n]
#define SYSCALL_ARG(n) SYSCALL_REG(I386_TCB_REG_EBX + (n))
#define SYSCALL_RET(n) SYSCALL_REG (I386_TCB_REG_EAX) = n
#define SYSCALL_FAIL(n)                         \
  {                                             \
    SYSCALL_RET (-n);                           \
    return;                                     \
  }

#define SYSCALL_RESOLVE_CAP_DEPTH(cap, cptr, depth)  \
  if ((cap = capslot_cspace_resolve (                \
         curr_tcb->cspace,                           \
         cptr,                                       \
         depth,                                      \
         NULL)) == NULL)                             \
    SYSCALL_FAIL (ATOMIK_ERROR_FAILED_LOOKUP)       

#define SYSCALL_RESOLVE_CAP(dest, cptr)         \
  SYSCALL_RESOLVE_CAP_DEPTH (dest, cptr, ATOMIK_FULL_DEPTH)

ASC_PROTO (cap_get_info)
{
  unsigned int depth = SYSCALL_ARG (1);
  capslot_t *cap;

  SYSCALL_RESOLVE_CAP_DEPTH (cap, SYSCALL_ARG (0), depth);

  SYSCALL_RET (cap->object_type);
  
  switch (cap->object_type)
  {
    case ATOMIK_OBJTYPE_UNTYPED:
      SYSCALL_REG (I386_TCB_REG_EBX) = cap->ut.size_bits;
      SYSCALL_REG (I386_TCB_REG_ECX) = cap->ut.access;
      SYSCALL_REG (I386_TCB_REG_ESI) =
        (uint32_t) cap->ut.base;
      SYSCALL_REG (I386_TCB_REG_EDI) = cap->ut.watermark;
      break;

    case ATOMIK_OBJTYPE_POOL:
      SYSCALL_REG (I386_TCB_REG_EBX) = cap->pool.object_size_bits;
      SYSCALL_REG (I386_TCB_REG_ECX) =
        cap->pool.access | (cap->pool.pool_type);
      SYSCALL_REG (I386_TCB_REG_EDX) =
        (uint32_t) cap->pool.size;
      SYSCALL_REG (I386_TCB_REG_ESI) =
        (uint32_t) cap->pool.base;
      SYSCALL_REG (I386_TCB_REG_EDX) =
        (uint32_t) cap->pool.available;
      break;
      
    case ATOMIK_OBJTYPE_CNODE:
      SYSCALL_REG (I386_TCB_REG_EBX) = cap->cnode.size_bits;
      SYSCALL_REG (I386_TCB_REG_ECX) =
        cap->cnode.access | (cap->cnode.guard_bits << 8);
      SYSCALL_REG (I386_TCB_REG_EDX) = cap->cnode.guard;
      break;
    
    case ATOMIK_OBJTYPE_TCB:
      SYSCALL_REG (I386_TCB_REG_ECX) = cap->tcb.access;
      break;

    case ATOMIK_OBJTYPE_PAGE:
    case ATOMIK_OBJTYPE_PT:
      SYSCALL_REG (I386_TCB_REG_EDI) =
        __atomik_capslot_get_page_vaddr (cap);

    case ATOMIK_OBJTYPE_PD:
      SYSCALL_REG (I386_TCB_REG_ECX) = cap->page.access;
      SYSCALL_REG (I386_TCB_REG_ESI) =
        (uint32_t) cap->page.base;
      break;    
  }
}

ASC_PROTO (cap_delete)
{
  capslot_t *cap;

  SYSCALL_RESOLVE_CAP (cap, SYSCALL_ARG (0));

  SYSCALL_RET (atomik_capslot_delete (cap));
}

ASC_PROTO (cap_revoke)
{
  capslot_t *cap;

  SYSCALL_RESOLVE_CAP (cap, SYSCALL_ARG (0));

  SYSCALL_RET (atomik_capslot_revoke (cap));
}

ASC_PROTO (cap_drop)
{
  capslot_t *cap;

  SYSCALL_RESOLVE_CAP (cap, SYSCALL_ARG (0));

  SYSCALL_RET (atomik_capslot_drop (cap, SYSCALL_ARG (1)));
}

ASC_PROTO (ut_retype)
{
  objtype_t type     = (uint8_t) SYSCALL_ARG (1);
  uint8_t size_bits  = SYSCALL_ARG (1) >> 8;
  uint8_t depth      = SYSCALL_ARG (1) >> 16;
  uintptr_t dest_off = SYSCALL_ARG (3); /* Offset in CNode */
  uintptr_t count    = SYSCALL_ARG (4);

  capslot_t *ut;
  capslot_t *dest;

  SYSCALL_RESOLVE_CAP (ut, SYSCALL_ARG (0));
  SYSCALL_RESOLVE_CAP_DEPTH (dest, SYSCALL_ARG (2), depth);
  
  if (dest->object_type != ATOMIK_OBJTYPE_CNODE)
    SYSCALL_FAIL (ATOMIK_ERROR_INVALID_CAPABILITY);

  if (dest_off + count > CNODE_COUNT (dest))
    SYSCALL_FAIL (ATOMIK_ERROR_RANGE);
  
  SYSCALL_RET (
    atomik_untyped_retype (
      ut,
      type,
      size_bits,
      CNODE_BASE (dest) + dest_off,
      count));
}

ASC_PROTO (page_remap)
{
  capslot_t *cap;

  SYSCALL_RESOLVE_CAP (cap, SYSCALL_ARG (0));

  SYSCALL_RET (atomik_page_remap (cap, SYSCALL_ARG (1)));
}

ASC_PROTO (pt_map_page)
{
  uintptr_t vaddr = SYSCALL_ARG (2);
  
  capslot_t *pt;
  capslot_t *pg;

  SYSCALL_RESOLVE_CAP (pt, SYSCALL_ARG (0));
  SYSCALL_RESOLVE_CAP (pg, SYSCALL_ARG (1));
  
  SYSCALL_RET (atomik_pt_map_page (pt, pg, vaddr));
}

ASC_PROTO (pt_remap)
{
  capslot_t *cap;

  SYSCALL_RESOLVE_CAP (cap, SYSCALL_ARG (0));

  SYSCALL_RET (atomik_pt_remap (cap, SYSCALL_ARG (1)));
}

ASC_PROTO (pd_map_pt)
{
  capslot_t *pd;
  capslot_t *pt;

  SYSCALL_RESOLVE_CAP (pd, SYSCALL_ARG (0));
  SYSCALL_RESOLVE_CAP (pt, SYSCALL_ARG (1));
  
  SYSCALL_RET (atomik_pd_map_pagetable (pd, pt, SYSCALL_ARG (2)));
}

ASC_PROTO (tcb_config)
{
  struct tcb_data *td;
  capslot_t *tcb;
  capslot_t *f_ep;
  capslot_t *croot;
  capslot_t *vroot;
  capslot_t *ipc;
  int ret;
  
  td = (struct tcb_data *) SYSCALL_ARG (1);
  
  if (!vspace_can_read (curr_vspace, td, sizeof (struct tcb_data)))
    SYSCALL_FAIL (ATOMIK_ERROR_INVALID_ADDRESS);
  
  SYSCALL_RESOLVE_CAP (tcb,   SYSCALL_ARG (0));
  SYSCALL_RESOLVE_CAP (f_ep,  td->td_fep);
  SYSCALL_RESOLVE_CAP_DEPTH (croot, td->td_croot, td->td_depth);
  SYSCALL_RESOLVE_CAP (vroot, td->td_vroot);
  SYSCALL_RESOLVE_CAP (ipc,   td->td_ipc);

  if ((ret = atomik_tcb_set_regs (tcb, &td->td_regs)) < 0)
    SYSCALL_FAIL (-ret);
  
  SYSCALL_RET (
    atomik_tcb_configure (
      tcb,
      f_ep,
      td->td_prio,
      croot,
      vroot,
      ipc));
}

ASC_PROTO (sched_push)
{
  capslot_t *tcb;

  SYSCALL_RESOLVE_CAP (tcb, SYSCALL_ARG (0));
  
  SYSCALL_RET (atomik_sched_push_tcb (tcb));
}

ASC_PROTO (sched_pull)
{
  capslot_t *tcb;

  SYSCALL_RESOLVE_CAP (tcb, SYSCALL_ARG (0));
  
  SYSCALL_RET (atomik_sched_pull_tcb (tcb));
}

ASC_PROTO (sched_yield)
{
  atomik_sched_schedule ();
}

void
i386_handle_syscall (unsigned int syscallno)
{
  switch (syscallno)
  {
    ASC_CASE (cap_get_info);
    ASC_CASE (cap_delete);
    ASC_CASE (cap_revoke);
    ASC_CASE (cap_drop);
    ASC_CASE (ut_retype);
    ASC_CASE (page_remap);
    ASC_CASE (pt_map_page);
    ASC_CASE (pt_remap);
    ASC_CASE (pd_map_pt);
    ASC_CASE (tcb_config);
    ASC_CASE (sched_push);
    ASC_CASE (sched_pull);
    ASC_CASE (sched_yield);
    
    case __ASC_d_putc:
      __arch_debug_putchar (SYSCALL_ARG (0));
      break;
    
    case __ASC_d_halt:
      __arch_machine_halt ();
      break;
    
    default:
      SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
  }
}
