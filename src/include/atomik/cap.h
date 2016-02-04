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
#  define ATOMIK_CAPSLOT_SIZE_BITS 5
#elif defined (__x86_64__)
#  define ATOMIK_CAPSLOT_SIZE_BITS 6
#else
#  error Unsupported architecture
#endif

#define ATOMIK_MIN_UT_SIZE_BITS    ATOMIK_CAPSLOT_SIZE_BITS
#define ATOMIK_CAPSLOT_SIZE (1 << ATOMIK_CAPSLOT_SIZE_BITS)
#define ATOMIK_OBJPART_SIZE (1 << (ATOMIK_CAPSLOT_SIZE_BITS - 1))

#define ATOMIK_ACCESS_EXEC  1
#define ATOMIK_ACCESS_WRITE 2
#define ATOMIK_ACCESS_READ  4
#define ATOMIK_ACCESS_GRANT 8

#define UT_BASE(utp) (utp)->ut.base
#define UT_SIZE(utp) (1 << (utp)->ut.size_bits)

#define CNODE_BASE(cnodep) (cnodep)->cnode.base
#define CNODE_SIZE(cnodep) (1 << ((cnodep)->cnode.size_bits + ATOMIK_CAPSLOT_SIZE_BITS))
#define CNODE_COUNT(cnodep) (1 << (cnodep)->cnode.size_bits)
#define CNODE_GUARD(cnodep) (cnodep)->cnode.guard
#define CNODE_GUARD_BITS(cnodep) (cnodep)->cnode.guard_bits

enum objtype
{
  ATOMIK_OBJTYPE_NULL,
  ATOMIK_OBJTYPE_UNTYPED,
  ATOMIK_OBJTYPE_CNODE,
  ATOMIK_OBJTYPE_PAGE,
  ATOMIK_OBJTYPE_PT,
  ATOMIK_OBJTYPE_PD
};

typedef enum objtype objtype_t;

struct capslot
{
  union
  {
    objtype_t object_type:8;

    /* Capability as untyped memory */
    struct
    {
      objtype_t object_type:8;
      uint8_t   size_bits;
      uint8_t   access;
      uint8_t   unused;
      void     *base;
    }
    ut;

    /* Capability as CNode */
    struct
    {
      objtype_t object_type:8;
      uint8_t size_bits; /* Size in log(entries) */
      uint8_t access;
      uint8_t guard_bits;
      struct capslot *base;
      uint32_t guard;

    }
    cnode;
    
    /* Capability as page */
    struct
    {
      objtype_t object_type:8;
      uint8_t   unused_1;
      uint8_t   access;
      uint8_t   unused_2;
      void     *base;
      struct capslot *pt; /* Backpointer to page table */
      uint16_t  entry; /* Entry in page table */
    }
    page;

    /* Capability as page table */
    struct
    {
      objtype_t object_type:8;
      uint8_t   unused_1;
      uint8_t   access;
      uint8_t   unused_2;
      uintptr_t *base;
      struct capslot *pd; /* Backpointer to page table */
      uint16_t  entry; /* Entry in page directory */
    }
    pt;

    /* Capability as page directory */
    struct
    {
      objtype_t object_type:8;
      uint8_t   unused_1;
      uint8_t   access;
      uint8_t   unused_2;
      uintptr_t *base;
    }
    pd;

    uint8_t pad[ATOMIK_OBJPART_SIZE];
  };

  struct capslot *mdb_parent;
  struct capslot *mdb_child;
  struct capslot *mdb_prev;
  struct capslot *mdb_next;
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

void capslot_clear (capslot_t *);

capslot_t *capslot_lookup (capslot_t *, cptr_t, unsigned char, struct caplookup_exception_info *);

void capabilities_init (capslot_t *);

int
atomik_untyped_retype (
    capslot_t *ut,
    objtype_t type,
    unsigned int size_bits,
    capslot_t *destination,
    unsigned int count);

int atomik_capslot_delete (capslot_t *);
int atomik_capslot_revoke (capslot_t *);

#endif /* _ATOMIK_CAP_H */
