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

/* To be implemented by architecture code */
int __cap_get_info (cptr_t, uint8_t, struct capinfo *);
int __cap_delete (cptr_t);
int __cap_revoke (cptr_t);
int __cap_drop (cptr_t);
int __ut_retype (
  cptr_t,
  objtype_t,
  uintptr_t,
  uintptr_t,
  cptr_t,
  uintptr_t,
  unsigned int);

int __pool_retype (cptr_t, objtype_t, unsigned int);
int __pool_alloc (cptr_t, cptr_t, unsigned int, size_t);
int __pool_free (cptr_t, void *);

#endif /* _SYSTEM_CALL_H */
