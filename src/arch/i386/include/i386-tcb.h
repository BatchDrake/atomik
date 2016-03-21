/*
 *    i386-tcb.h: i386 TCB definitions
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

#ifndef _ARCH_I386_TCB_H
#define _ARCH_I386_TCB_H

#define I386_TCB_REG_NUM 13

enum i386_reg_id
{
  I386_TCB_REG_EAX,
  I386_TCB_REG_EBX,
  I386_TCB_REG_ECX,
  I386_TCB_REG_EDX,
  I386_TCB_REG_ESI,
  I386_TCB_REG_EDI,
  I386_TCB_REG_EBP,
  I386_TCB_REG_ESP,
  I386_TCB_REG_EFLAGS,
  I386_TCB_REG_EIP
};

typedef enum i386_reg_id i386_reg_id_t;

struct i386_tcb_regs
{
  uint32_t r[I386_TCB_REG_NUM];
};

struct tcb_data
{
  cptr_t  td_tcb;
  cptr_t  td_fep;
  cptr_t  td_croot;
  cptr_t  td_vroot;
  cptr_t  td_ipc;

  uint8_t td_prio;
  uint8_t td_depth;

  struct i386_tcb_regs td_regs;
};

typedef struct i386_tcb_regs tcb_regs_t;

#endif /* _ARCH_I386_TCB_H */
