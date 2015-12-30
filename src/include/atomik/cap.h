/*
 *    cap.h: Atomik capabilities
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

#ifndef _ATOMIK_CAP_H
#define _ATOMIK_CAP_H

#define __FILE_ID atomik_cap_h

#include <util.h>
#include <alltypes.h>

#define CPTR_BITS 32

#if defined (__i386__)
#  define ATOMIK_CAPSLOT_SIZE_BITS 4
#  define ATOMIK_CAPSLOT_ADDR_BITS 28
#elif defined (__x86_64__)
#  define ATOMIK_CAPSLOT_SIZE_BITS 5
#  define ATOMIK_CAPSLOT_ADDR_BITS 60
#else
#  error Unsupported architecture
#endif

#define ATOMIK_CAPSLOT_SIZE (1 << ATOMIK_CAPSLOT_SIZE_BITS)
#define ATOMIK_CAPSLOT_GET_OBJECT_ADDR(capslot) \
  ((void *) ((capslot)->base << 4))
#define ATOMIK_CAPSLOT_SET_OBJECT_ADDR(capslot, addr)   \
  (capslot)->base = (uintptr_t) addr >> 4;

enum objtype
{
  ATOMIK_OBJTYPE_NULL,
  ATOMIK_OBJTYPE_UNTYPED,
  ATOMIK_OBJTYPE_CNODE
};

typedef enum objtype objtype_t;

/* Capability slot type. Should be exactly 16 bytes long */
struct capslot
{
  objtype_t object_type:4;  /* Up to 16 object types */
  
  uintptr_t base:ATOMIK_CAPSLOT_ADDR_BITS;

  union
  {
    /* Capability as untyped memory */
    struct
    {
      uint8_t size_bits:6; /* Size in log(bytes) */
    }
    ut;

    /* Capability as CNode */
    struct
    {
      uint8_t size_bits:5; /* Size in log(entries) */      
      uint8_t guard_bits:4;
      uint16_t guard;
    }
    cnode;
    
    /* Capability as page */
    struct
    {
      uint8_t access:3;
    }
    page;
  };
  
  uintptr_t mdb_prev:ATOMIK_CAPSLOT_ADDR_BITS;
  uintptr_t mdb_next:ATOMIK_CAPSLOT_ADDR_BITS;
};

typedef struct capslot capslot_t;

CPPASSERT(sizeof (capslot_t) == ATOMIK_CAPSLOT_SIZE);

typedef uint32_t cptr_t;

enum caplookup_exception
{
  ATOMIK_CAPLOOKUP_EXCEPTION_SUCCESS,
  ATOMIK_CAPLOOKUP_EXCEPTION_INVALID_ROOT,
  ATOMIK_CAPLOOKUP_EXCEPTION_MISSING_CAPABILITY,
  ATOMIK_CAPLOOKUP_EXCEPTION_DEPTH_MISMATCH,
  ATOMIK_CAPLOOKUP_EXCEPTION_GUARD_MISMATCH
};

typedef enum caplookup_exception caplookup_exception_t;

struct caplookup_exception_info
{
  caplookup_exception_t exception;
  uint8_t bits_left;

  union
  {
    uint8_t bits_resolved;
    uint32_t guard;
  };

  uint8_t guard_bits;
};

void cnode_init (capslot_t *);

capslot_t *capslot_lookup (capslot_t *, cptr_t, unsigned char, struct caplookup_exception_info *);

void capabilities_init (capslot_t *);

#endif /* _ATOMIK_CAP_H */
