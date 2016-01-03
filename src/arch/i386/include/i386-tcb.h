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

struct i386_tcb_regs
{
  uint32_t r[I386_TCB_REG_NUM];
};

typedef struct i386_tcb_regs tcb_regs_t;

#endif /* _ARCH_I386_TCB_H */
