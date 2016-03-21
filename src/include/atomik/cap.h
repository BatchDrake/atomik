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

#if defined (__i386__)
#  define ATOMIK_CAPSLOT_SIZE_BITS 5
#elif defined (__x86_64__)
#  define ATOMIK_CAPSLOT_SIZE_BITS 6
#else
#  error Unsupported architecture
#endif

#define ATOMIK_PAGE_SIZE_BITS PAGE_BITS
#define ATOMIK_PT_SIZE_BITS   PT_BITS
#define ATOMIK_PD_SIZE_BITS   PD_BITS
#define ATOMIK_TCB_SIZE_BITS  7

#define ATOMIK_MIN_UT_SIZE_BITS    ATOMIK_CAPSLOT_SIZE_BITS
#define ATOMIK_CAPSLOT_SIZE BIT (ATOMIK_CAPSLOT_SIZE_BITS)
#define ATOMIK_OBJPART_SIZE BIT ((ATOMIK_CAPSLOT_SIZE_BITS - 1))

#define ATOMIK_ACCESS_EXEC  1
#define ATOMIK_ACCESS_WRITE 2
#define ATOMIK_ACCESS_READ  4
#define ATOMIK_ACCESS_GRANT 8
#define ATOMIK_ACCESS_REMAP 16

#define ATOMIK_FULL_DEPTH   0xff

#define ATOMIK_FULL_ACCESS \
  (ATOMIK_ACCESS_EXEC  | \
   ATOMIK_ACCESS_WRITE | \
   ATOMIK_ACCESS_READ  | \
   ATOMIK_ACCESS_GRANT)

#define UT_BASE(utp) (utp)->ut.base
#define UT_SIZE(utp) BIT ((utp)->ut.size_bits)

#define CNODE_BASE(cnodep) (cnodep)->cnode.base
#define CNODE_SIZE(cnodep) BIT ((cnodep)->cnode.size_bits + ATOMIK_CAPSLOT_SIZE_BITS)
#define CNODE_COUNT(cnodep) BIT ((cnodep)->cnode.size_bits)
#define CNODE_GUARD(cnodep) (cnodep)->cnode.guard
#define CNODE_GUARD_BITS(cnodep) (cnodep)->cnode.guard_bits

#define POOL_BASE(poolp) (poolp)->pool.base

#define PAGE_BASE(pagep) (pagep)->page.base

#define capslot_t_INITIALIZER \
  { ATOMIK_OBJTYPE_NULL }
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
  ATOMIK_OBJTYPE_TCB,
  ATOMIK_OBJTYPE_POOL
};

typedef enum objtype objtype_t;

enum epstate
{
  ATOMIK_EPSTATE_IDLE,
  ATOMIK_EPSTATE_SEND,
  ATOMIK_EPSTATE_RECV
};

typedef enum epstate epstate_t;

struct tcb;

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
      uint8_t   unused;    /* Size of objects inside this UT */
      void     *base;
      size_t    watermark; /* Untyped memory watermark level */
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
      struct tcb *tcb; /* Backpointer to TCB */
    }
    cnode;
    
    /* Capability as TCB */
    struct
    {
      objtype_t object_type:8;
      uint8_t unused_1;
      uint8_t access;
      uint8_t unused_2;
      struct tcb *base;
    }
    tcb;

    /* Capability as object pool */
    struct
    {
      objtype_t object_type:8;
      uint8_t   object_size_bits; /* Refers to each particular object */
      uint8_t   access;
      objtype_t pool_type:8; /* Type of the objects within */
      void     *base; /* Physical address */
      size_t    size; /* Allocation size in objects (bytes if untyped) */
      size_t    available; /* Available objects */
    }
    pool;

#ifdef __i386__
    /* Capability as page */
    struct
    {
      objtype_t object_type:8;
      uint8_t   vaddr_lo1; /* Hello 80286 */
      uint8_t   access;
      uint8_t   vaddr_lo2;
      void     *base;
      struct capslot *pt; /* Backpointer to page table */
      uint16_t  vaddr_hi;
    }
    page;

    /* Capability as page table */
    struct
    {
      objtype_t object_type:8;
      uint8_t   vaddr_lo1;
      uint8_t   access;
      uint8_t   vaddr_lo2;
      uintptr_t *base;    /* Must be remapped */
      struct capslot *pd; /* Backpointer to page table */
      uint16_t  vaddr_hi; /* Entry in page directory */
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
      struct tcb *tcb;
    }
    pd;
#else
# error "Page capabilities not defined for 64 bits"
#endif
    /* Capability as endpoint */
    struct
    {
      objtype_t object_type:8;
      epstate_t state:8;
      uint8_t   access;
      uint8_t   unused;
      uint32_t  badge;
      struct tcb *wq_head;
      struct tcb *wq_tail;
    }
    ep;

    /* Generic objects (to access common fields) */
    struct
    {
      objtype_t object_type:8;
      uint8_t   size_bits;
      uint8_t   access;
      uint8_t   unused;
      void     *base;
    }
    generic;

    uint8_t pad[ATOMIK_OBJPART_SIZE];
  };

  struct capslot *mdb_parent;
  struct capslot *mdb_child;
  struct capslot *mdb_prev;
  struct capslot *mdb_next;
};

typedef struct capslot capslot_t;

CPPASSERT(sizeof (capslot_t) == ATOMIK_CAPSLOT_SIZE);

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

static inline error_t
__atomik_objtype_adjust_size_bits (objtype_t type, unsigned int *size_bits)
{
  error_t exception = ATOMIK_SUCCESS;

  switch (type)
  {
    case ATOMIK_OBJTYPE_UNTYPED:
      if (*size_bits < ATOMIK_MIN_UT_SIZE_BITS)
        ATOMIK_FAIL (ATOMIK_ERROR_INVALID_SIZE);
      break;

    case ATOMIK_OBJTYPE_CNODE:
      *size_bits += ATOMIK_CAPSLOT_SIZE_BITS;
      break;

    case ATOMIK_OBJTYPE_PAGE:
      *size_bits = ATOMIK_PAGE_SIZE_BITS;
      break;

    case ATOMIK_OBJTYPE_PD:
      *size_bits = ATOMIK_PD_SIZE_BITS;
      break;

    case ATOMIK_OBJTYPE_PT:
      *size_bits = ATOMIK_PT_SIZE_BITS;
      break;

    case ATOMIK_OBJTYPE_TCB:
      *size_bits = ATOMIK_TCB_SIZE_BITS;
      break;

    case ATOMIK_OBJTYPE_ENDPOINT:
      *size_bits = 2;
      break;

    case ATOMIK_OBJTYPE_POOL:
      break;
      
    default:
      ATOMIK_FAIL (ATOMIK_ERROR_INVALID_TYPE);
  }

fail:

  return exception;
}

/*
 * System calls
 */
int
atomik_untyped_retype (
    capslot_t *ut,
    objtype_t type,
    unsigned int size_bits,
    capslot_t *destination,
    unsigned int count);

int atomik_capslot_delete (capslot_t *);

int atomik_capslot_revoke (capslot_t *);

int atomik_capslot_drop (capslot_t *, uint8_t);

int atomik_pool_retype (capslot_t *, objtype_t, unsigned int);

int atomik_pool_alloc (capslot_t *, capslot_t *, size_t);

int atomik_pool_free (capslot_t *, void *);

/*
 * Convenience functions
 */
error_t capslot_init (capslot_t *, objtype_t, size_t, uint8_t, void *);

void capslot_add_child (capslot_t *, capslot_t *);

capslot_t *capslot_cspace_resolve (capslot_t *, cptr_t, unsigned char, struct caplookup_exception_info *);

void capslot_clear (capslot_t *);

void capabilities_init (capslot_t *);

error_t pool_free (capslot_t *, void *);

void pool_init (void);

int pool_revoke (capslot_t *);

#endif /* _ATOMIK_CAP_H */
