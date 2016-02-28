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

ASC_PROTO (cap_get_info)
{
  cptr_t cptr = (cptr_t) SYSCALL_ARG (0);
  unsigned int depth =   SYSCALL_ARG (1);
  capslot_t *cap;
  
  if ((cap = capslot_cspace_resolve (
         curr_tcb->cspace,
         cptr,
         depth,
         NULL)) == NULL)
    SYSCALL_FAIL (ATOMIK_ERROR_FAILED_LOOKUP);

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
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (cap_revoke)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (cap_drop)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (ut_retype)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (page_remap)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (pt_map_page)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (pt_remap)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (pd_map_pt)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (tcb_config)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (sched_push)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (sched_pull)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
}

ASC_PROTO (sched_yield)
{
  SYSCALL_RET (-ATOMIK_ERROR_INVALID_SYSCALL);
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
