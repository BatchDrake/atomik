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

#define __ASC_d_halt 0xfffffffe
#define __ASC_d_putc 0xffffffff

void d_putch (char);
void d_halt (void);

#endif /* _ATOMIK_USER_H */
