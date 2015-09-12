/*
 *    i386-regs.h: i386 register definitions
 *    Copyright (C) 2014  Gonzalo J. Carracedo
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

#ifndef _ARCH_I386_REGS_H
#define _ARCH_I386_REGS_H


#define GET_REGISTER(reg, where)                \
  __asm__ __volatile__ ("mov %" reg ", %%eax\n" \
                        "mov %%eax, %0" : "=g" (where) :: "eax");

#define SET_REGISTER(reg, where)                \
  __asm__ __volatile__ ("mov %0, %%eax\n"       \
                        "mov %%eax, %" reg :: "g" (where) : "eax");
  
#define CR0_PAGING_ENABLED 0x80000000

/* Extended processor flags to use with EFLAGS */

#define EFLAGS_CARRY     (1 <<  0)
#define EFLAGS_PARITY    (1 <<  2)
#define EFLAGS_ADJUST    (1 <<  4)
#define EFLAGS_ZERO      (1 <<  6)
#define EFLAGS_SIGN      (1 <<  7)
#define EFLAGS_TRAP      (1 <<  8)
#define EFLAGS_INTERRUPT (1 <<  9)
#define EFLAGS_DIRECTION (1 << 10)
#define EFLAGS_OVERFLOW  (1 << 11)
#define EFLAGS_NESTED    (1 << 14)
#define EFLAGS_RESUME    (1 << 16)
#define EFLAGS_VM8086    (1 << 17)
#define EFLAGS_ALIGNMENT (1 << 18)
#define EFLAGS_VIRT_INTR (1 << 19)
#define EFLAGS_INT_PENDG (1 << 20)
#define EFLAGS_IDENTIFY  (1 << 21)

struct x86_common_regs
{
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
};

struct x86_segment_regs
{
  uint32_t cs;
  uint32_t ds;
  uint32_t es;
  uint32_t gs;
  uint32_t fs;
  uint32_t ss;
};

struct x86_int_frame
{
  uint32_t error;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
};

struct x86_int_frame_privchg
{
  uint32_t error;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t old_esp;
  uint32_t old_ss;
};

struct x86_stack_frame
{
  struct x86_segment_regs segs;
  uint32_t                cr0;
  uint32_t                cr3;
  struct x86_common_regs  regs;
  uint32_t                int_no;
  union
  {
    struct x86_int_frame         priv;
    struct x86_int_frame_privchg unpriv;
  };
};

#endif /* _ARCH_I386_REGS_H */
