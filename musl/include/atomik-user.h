/*
 *    atomik-user.h: Atomik system call interface
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

#ifndef _ATOMIK_USER_H
#define _ATOMIK_USER_H

#define AOT_ANY     0
#define AOT_UNTYPED 1
#define AOT_CNODE   2
#define AOT_PAGE    3
#define AOT_PT      4
#define AOT_PD      5
#define AOT_EP      6
#define AOT_NOTIFY  7
#define AOT_TCB     8
#define AOT_POOL    9

#define ASC_NO(aot, func) \
  (((aot) << 24) | (func))

#define __ASC_cap_get_info ASC_NO (AOT_ANY, 0)
#define __ASC_cap_delete   ASC_NO (AOT_ANY, 1)
#define __ASC_cap_revoke   ASC_NO (AOT_ANY, 2)
#define __ASC_cap_drop     ASC_NO (AOT_ANY, 3)

#define __ASC_ut_retype    ASC_NO (AOT_UNTYPED, 1)

#define __ASC_page_remap   ASC_NO (AOT_PAGE, 1)

#define __ASC_pt_map_page  ASC_NO (AOT_PT, 0)
#define __ASC_pt_remap     ASC_NO (AOT_PT, 1)

#define __ASC_pd_map_pt    ASC_NO (AOT_PD, 0)

#define __ASC_tcb_config   ASC_NO (AOT_TCB, 0)
#define __ASC_sched_push   ASC_NO (AOT_TCB, 1)
#define __ASC_sched_pull   ASC_NO (AOT_TCB, 2)
#define __ASC_sched_yield  ASC_NO (AOT_TCB, 3)

#define __ASC_pool_retype  ASC_NO (AOT_POOL, 0)
#define __ASC_pool_alloc   ASC_NO (AOT_POOL, 1)
#define __ASC_pool_free    ASC_NO (AOT_POOL, 2)

#define __ASC_d_halt 0xfffffffe
#define __ASC_d_putc 0xffffffff

void d_putch (char);
void d_halt (void);

#endif /* _ATOMIK_USER_H */
