/*
 *    call.h: Atomik system call interface
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

#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

/* Includes from musl */
#include <alltypes.h>
#include <atomik-user.h> 

struct capinfo;

/* To be implemented by architecture code */
int __cap_get_info (cptr_t, uint8_t, struct capinfo *);

#endif /* _SYSTEM_CALL_H */
