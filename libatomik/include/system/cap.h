/*
 *    cap.h: Capability system calls
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

#ifndef _SYSTEM_CAP_H
#define _SYSTEM_CAP_H

#include <alltypes.h>
#include <atomik-user.h> 
#include <errno.h>

#define ATOMIK_FULL_DEPTH   0xff

#define ATOMIK_ACCESS_EXEC  1
#define ATOMIK_ACCESS_WRITE 2
#define ATOMIK_ACCESS_READ  4
#define ATOMIK_ACCESS_GRANT 8
#define ATOMIK_ACCESS_REMAP 16

enum objtype
{
  ATOMIK_OBJTYPE_NULL,
  ATOMIK_OBJTYPE_UNTYPED,
  ATOMIK_OBJTYPE_CNODE,
  ATOMIK_OBJTYPE_PAGE,
  ATOMIK_OBJTYPE_PT,
  ATOMIK_OBJTYPE_PD,
  ATOMIK_OBJTYPE_ENDPOINT,
  ATOMIK_OBJTYPE_NOTIFICATION,
  ATOMIK_OBJTYPE_TCB
};
typedef enum objtype objtype_t;

struct capinfo
{
  objtype_t ci_type;
  uint8_t   ci_bits;
  uint8_t   ci_access;
  uint8_t   ci_guard_bits;
  uint32_t  ci_guard;
  uint32_t  ci_paddr;
  
  union
  {
    uint32_t ci_vaddr;
    uint32_t ci_watermark;
  };
};

#include <system/call.h>

int cap_get_info (cptr_t, uint8_t, struct capinfo *);

#endif /* _SYSTEM_CAP_H */
