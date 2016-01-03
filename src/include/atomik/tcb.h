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

struct tcb
{
  /* Context registers must be at offset 0 */
  tcb_regs_t regs;
  
  capslot_t cspace;
  cptr_t    vspace;
};

typedef struct tcb tcb_t;

#endif /* _ATOMIK_TCB_H */
