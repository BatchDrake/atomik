/*
 *    elf.c: Create TCBs out of embedded ELF files.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atomik/atomik.h>
#include <atomik/cap.h>
#include <atomik/vspace.h>
#include <atomik/tcb.h>
#include <atomik/elf.h>

#define ELF32_TASK_TCB_STACK_PAGES 16
#define ELF32_TASK_TCB_STACK_PTS   1

#define ELF32_MSG(fmt, arg...) printf ("elf32: " fmt "\n", ##arg)
#define ELF32_THROW(fmt, arg...)             \
  {                                          \
    ELF32_MSG("exception: " fmt, ##arg);     \
    goto fail;                               \
  }

struct elf32_handle
{
  Elf32_Ehdr *ehdr;
  Elf32_Phdr *phdrs;
  unsigned int phdr_count;
  void *base;
  size_t size;
};

typedef struct elf32_handle elf32_handle_t;

struct bufstate
{
  uint8_t *base;
  unsigned int ptr;
  unsigned int size;
};

void
bufstate_init (struct bufstate *state, void *base, size_t size)
{
  state->base = (uint8_t *) base;
  state->ptr  = 0;
  state->size = size;
}

void *
bufstate_read (struct bufstate *state, size_t size)
{
  void *retval;

  if (state->ptr + size > state->size)
    return NULL;

  retval = state->base + state->ptr;

  state->ptr += size;

  return retval;
}

int
bufstate_seek (struct bufstate *state, unsigned int off)
{
  if (state->ptr + off > state->size)
    return 0;

  state->ptr = off;

  return 1;
}

static inline int
elf32_header_is_valid (Elf32_Ehdr *ehdr)
{
  if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
      ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
      ehdr->e_ident[EI_MAG3] != ELFMAG3)
    ELF32_THROW ("Invalid ELF header");

  if (ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
      ehdr->e_ident[EI_DATA]  != ELFDATA2LSB ||
      ehdr->e_machine != EM_386)
    ELF32_THROW ("Invalid architecture (0x%x, 0x%x, 0x%x)",
                 ehdr->e_ident[EI_CLASS],
                 ehdr->e_ident[EI_DATA],
                 ehdr->e_machine);

  return 1;

fail:
  return 0;
}

int
elf32_init (elf32_handle_t *handle, void *buf, size_t size)
{
  struct bufstate bs;
  uint8_t *as_bytes = (uint8_t *) buf;
  unsigned int i;

  handle->base = buf;
  handle->size = size;

  bufstate_init (&bs, buf, size);

  if ((handle->ehdr = bufstate_read (&bs, sizeof (Elf32_Ehdr))) == NULL)
    ELF32_THROW ("Unexpected EOF while reading executable header");

  if (!elf32_header_is_valid (handle->ehdr))
    ELF32_THROW ("Invalid ELF file");

  if (!bufstate_seek (&bs, handle->ehdr->e_phoff))
    ELF32_THROW ("Cannot seek to PHDR table");

  handle->phdr_count = handle->ehdr->e_phnum;

  printf ("In header: %d entries\n", handle->ehdr->e_phnum);

  if ((handle->phdrs = bufstate_read (
      &bs,
      sizeof (Elf32_Phdr) * handle->phdr_count)) == NULL)
    ELF32_THROW ("Cannot seek to PHDR table");

  for (i = 0; i < handle->phdr_count; ++i)
    if (handle->phdrs[i].p_type == PT_LOAD)
    {
      if (!bufstate_seek (&bs, handle->phdrs[i].p_offset))
        ELF32_THROW ("Cannot seek to PHDR contents");
      if (!bufstate_read(&bs, handle->phdrs[i].p_filesz))
        ELF32_THROW ("Unexpected EOF while reading PT_LOAD contents");
    }

  return 1;

fail:
  return 0;
}

void
elf32_debug (const elf32_handle_t *handle)
{
  unsigned int i;

  ELF32_MSG ("Executable contents:");

  for (i = 0; i < handle->phdr_count; ++i)
    if (handle->phdrs[i].p_type == PT_LOAD)
      ELF32_MSG (
          " (%02x) 0x%08x - 0x%08x [0x%x]",
          i,
          handle->phdrs[i].p_vaddr,
          handle->phdrs[i].p_vaddr + handle->phdrs[i].p_memsz - 1,
          handle->phdrs[i].p_flags);
}

Elf32_Phdr *
elf32_find_phdr (const elf32_handle_t *handle, unsigned int mask)
{
  unsigned int i;

  for (i = 0; i < handle->phdr_count; ++i)
    if (handle->phdrs[i].p_type == PT_LOAD &&
       (handle->phdrs[i].p_flags & mask) == mask)
      return &handle->phdrs[i];

  return NULL;
}

static capslot_t *
elf32_find_small_ut (const capslot_t *root, unsigned int bits)
{
  unsigned int count;
  unsigned int i;
  unsigned int smallest = 32;

  capslot_t *found = NULL;

  count = CNODE_COUNT (root);

  for (i = 0; i < count; ++i)
    if (root->cnode.base[i].object_type == ATOMIK_OBJTYPE_UNTYPED)
    {
      if (root->cnode.base[i].ut.size_bits >= bits &&
          root->cnode.base[i].ut.size_bits < smallest)
      {
        smallest = root->cnode.base[i].ut.size_bits;
        found = &root->cnode.base[i];

        if (smallest == bits)
          break;
      }
    }

  return found;
}

static capslot_t *
elf32_find_free_capslots (const capslot_t *root, unsigned int count)
{
  unsigned int i, j;
  unsigned int entries;

  entries = CNODE_COUNT (root);

  for (i = 0; i < entries; ++i)
    if (root->cnode.base[i].object_type == ATOMIK_OBJTYPE_NULL)
    {
      for (j = 0; j < count && (i + j) < entries; ++j)
        if (root->cnode.base[i + j].object_type != ATOMIK_OBJTYPE_NULL)
          break; /* Found busy capslot */

      if (j == count) /* Found `count' free capslots */
        break;

      i += j; /* [i + j] has already been visited */
    }

  if (i < entries)
    return &root->cnode.base[i];

  return NULL;
}

capslot_t *
elf32_load_tcb (void *buf, size_t size, capslot_t *croot)
{
  elf32_handle_t handle;
  Elf32_Phdr *codeseg, *dataseg, *seg;
  capslot_t *ut;
  capslot_t *tcb_desc_cnode;
  capslot_t *page_caps;
  capslot_t *pt_caps;
  capslot_t *pd_cap;
  capslot_t *tcb_cap;
  capslot_t *ep_cap;

  capslot_t *curr_pt = NULL;
  uintptr_t first_page, last_page;
  uintptr_t first_pt, last_pt;
  uintptr_t vaddr;
  uintptr_t attrib;

  unsigned int pages;
  unsigned int pts;
  unsigned int objcount;
  unsigned int allocsize;
  unsigned int i, j;
  size_t fittest_bits = 0;
  int retval;

  if (!elf32_init (&handle, buf, size))
    return NULL;

  elf32_debug (&handle);

  if ((codeseg = elf32_find_phdr (&handle, PF_R | PF_X)) == NULL)
    ELF32_THROW ("Cannot find code segment for root task");

  if ((dataseg = elf32_find_phdr (&handle, PF_R | PF_W)) == NULL)
    ELF32_THROW ("Cannot find data segment for root task");

  if (codeseg->p_vaddr > dataseg->p_vaddr)
    ELF32_THROW ("Code segment cannot be above data segment");

  if (dataseg->p_vaddr - (codeseg->p_vaddr + codeseg->p_memsz) > PAGE_SIZE)
    ELF32_THROW ("Code/data gap too big");

  if (codeseg->p_vaddr & PAGE_CONTROL_MASK)
    ELF32_THROW ("Unaligned code segment");

  first_page = PAGE_START (codeseg->p_vaddr);
  last_page  = PAGE_START (dataseg->p_vaddr + dataseg->p_memsz - 1);

  first_pt   = PT_START (codeseg->p_vaddr);
  last_pt    = PT_START (dataseg->p_vaddr + dataseg->p_memsz - 1);

  ELF32_MSG ("First page: 0x%x", first_page);
  ELF32_MSG ("Last page:  0x%x", last_page);
  ELF32_MSG ("First page table: 0x%x", first_pt);
  ELF32_MSG ("Last page table:  0x%x", last_pt);

  /* We cannot just add both memsz heres */
  pages = ((last_page - first_page) >> PAGE_BITS) + 1;
  pts   = ((last_pt - first_pt) >> (PAGE_BITS + PTE_BITS)) + 1;

  ELF32_MSG ("Memory requirements for root task:");
  ELF32_MSG (" Number of pages: %d", pages);
  ELF32_MSG (" Number of page tables: %d", pts);
  ELF32_MSG (" Number of stack pages: %d", ELF32_TASK_TCB_STACK_PAGES);
  ELF32_MSG (" Number of stack page tables: %d", ELF32_TASK_TCB_STACK_PTS);

  /*
   * Impose restriction: text segment and data segment must be separaed
   * no more than 1 page.
   * Number of data pages needed:  PT_LOAD pages + stack pages + threadinfo
   * Number of page tables needed: Highest address minus lowest address and
   *                               divide accordingly + stack pages divided
   *                               accordingly (threadinfo should go right
   *                               after the stack)
   * Number of PDs: 1
   * Number of TCBs: 1
   * Number of Fault EPs: 1
   *
   * Pages for stack: 16
   * Page tables for stack: 1
   */

  objcount = pages + pts + 3 +
      ELF32_TASK_TCB_STACK_PAGES + ELF32_TASK_TCB_STACK_PTS;


  /*
   * To allocate CNode for all thread structures: compute summation of the
   * above quantities, find the fittest power of two for CNode size, find the
   * fittest UT (not the worst) and retype in root CNode.
   *
   * Compute summation of all pages needed (including PTs) and retype,
   * do the same for all TCBs.
   */


  while (BIT (fittest_bits) < objcount)
    ++fittest_bits;

  /*
   * Minimizing fragmentation will depend solely on userland code. We will
   * try to find the fittest UT and allocate on it (note that at this
   * point there are no partially allocated UTs)
   */

  ELF32_MSG ("Find UT and retype CNode for %d objects", objcount);

  if ((ut = elf32_find_small_ut (
              croot,
              fittest_bits + ATOMIK_CAPSLOT_SIZE_BITS)) == NULL)
    ELF32_THROW (
        "Not enough memory for CNode of %d bytes (object caps)",
        BIT (fittest_bits + ATOMIK_CAPSLOT_SIZE_BITS));

  ELF32_MSG ("Find free capslots");

  if ((tcb_desc_cnode = elf32_find_free_capslots (croot, 1)) == NULL)
    ELF32_THROW ("Root CNode is full");

  ELF32_MSG ("Retype CNode of %d bytes", BIT (fittest_bits + ATOMIK_CAPSLOT_SIZE_BITS));

  if ((retval = atomik_untyped_retype (
                    ut,
                    ATOMIK_OBJTYPE_CNODE,
                    fittest_bits,
                    tcb_desc_cnode,
                    1)) < 0)
    ELF32_THROW ("Retype failed: %s", error_to_string (-retval));


  /* FIXME: The allocsize below is totally wrong. */
  allocsize = objcount * PAGE_SIZE;

  fittest_bits = 0;
  while (BIT (fittest_bits) < allocsize)
      ++fittest_bits;

  ELF32_MSG ("Allocate %d bytes for objects (%d required)",
             BIT (fittest_bits), allocsize);

  /* Allocate for pages, TCBs, etc */
  if ((ut = elf32_find_small_ut (croot, fittest_bits)) == NULL)
    ELF32_THROW (
            "Not enough memory for CNode of %d bytes (object contents)",
            BIT (fittest_bits));

  /* Caps, caps for everyone */
  page_caps = &CNODE_BASE (tcb_desc_cnode)[0];
  pt_caps   = page_caps + pages + ELF32_TASK_TCB_STACK_PAGES;
  pd_cap    = pt_caps + pts + ELF32_TASK_TCB_STACK_PTS;
  tcb_cap   = pd_cap + 1;
  ep_cap    = tcb_cap + 1;


  /* Retype everything */
  if ((retval = atomik_untyped_retype (
                      ut,
                      ATOMIK_OBJTYPE_PAGE,
                      0,
                      page_caps,
                      pages + ELF32_TASK_TCB_STACK_PAGES)) < 0)
      ELF32_THROW ("Retype failed (pages): %s", error_to_string (-retval));

  if ((retval = atomik_untyped_retype (
                        ut,
                        ATOMIK_OBJTYPE_PT,
                        0,
                        pt_caps,
                        pts + ELF32_TASK_TCB_STACK_PTS)) < 0)
    ELF32_THROW ("Retype failed (tables): %s", error_to_string (-retval));

  if ((retval = atomik_untyped_retype (
                        ut,
                        ATOMIK_OBJTYPE_PD,
                        0,
                        pd_cap,
                        1)) < 0)
    ELF32_THROW ("Retype failed (directory): %s", error_to_string (-retval));

  if ((retval = atomik_untyped_retype (
                        ut,
                        ATOMIK_OBJTYPE_TCB,
                        0,
                        tcb_cap,
                        1)) < 0)
    ELF32_THROW ("Retype failed (TCB): %s", error_to_string (-retval));

  if ((retval = atomik_untyped_retype (
                        ut,
                        ATOMIK_OBJTYPE_ENDPOINT,
                        0,
                        ep_cap,
                        1)) < 0)
    ELF32_THROW ("Retype failed (endpoint): %s", error_to_string (-retval));

  ELF32_MSG ("Mapping code & data segments...");

  /* Map code & data segments */
  for (i = 0; i < pages; ++i)
  {
    vaddr = first_page + PAGE_ADDRESS (i);

    if (curr_pt == NULL)
    {
      curr_pt = pt_caps;

      if ((retval = atomik_pd_map_pagetable (pd_cap, curr_pt, vaddr)) < 0)
        ELF32_THROW ("Map table failed: %s", error_to_string (-retval));
    }
    else if (vaddr == PT_START (vaddr))
    {
      ++curr_pt;

      if ((retval = atomik_pd_map_pagetable (pd_cap, curr_pt, vaddr)) < 0)
        ELF32_THROW ("Map table failed: %s", error_to_string (-retval));
    }

    if ((retval = atomik_pt_map_page (curr_pt, &page_caps[i], vaddr)) < 0)
      ELF32_THROW ("Map page failed: %s", error_to_string (-retval));

    if (vaddr >= codeseg->p_vaddr &&
        vaddr < codeseg->p_vaddr + codeseg->p_memsz)
    {
      attrib = ATOMIK_ACCESS_READ | ATOMIK_ACCESS_EXEC;
      seg    = codeseg;
    }
    else if (vaddr >= dataseg->p_vaddr &&
        vaddr < dataseg->p_vaddr + dataseg->p_memsz)
    {
      attrib = ATOMIK_ACCESS_READ | ATOMIK_ACCESS_WRITE;
      seg    = dataseg;
    }
    else
      seg = NULL;

    if ((retval = atomik_page_remap (&page_caps[i], attrib)) < 0)
      ELF32_THROW (
          "Adjust page attributes failed: %s",
          error_to_string (-retval));

    /*
     * Copy data. We can do this because we still have a 1:1 mapping of
     * physical memory.
     */

    if (seg != NULL &&
        vaddr >= seg->p_vaddr &&
        vaddr < seg->p_vaddr + seg->p_filesz)
      memcpy (
          page_caps[i].page.base,
          (const uint8_t *) buf + seg->p_offset + PAGE_ADDRESS (i),
          PAGE_SIZE);
    else
      memset (page_caps[i].page.base, 0, PAGE_SIZE);

  }

  ELF32_MSG ("Mapping stack segments...");

  /* Map stack segment */
  vaddr = PREFERED_STACK_BASE;
  curr_pt = NULL;

  for (i = 0; i < ELF32_TASK_TCB_STACK_PAGES; ++i)
  {
    vaddr = PREFERED_STACK_BASE + (i << PAGE_BITS);

    if (curr_pt == NULL)
    {
      curr_pt = &pt_caps[pts];

      if ((retval = atomik_pd_map_pagetable (pd_cap, curr_pt, vaddr)) < 0)
        ELF32_THROW ("Map table failed (stack): %s", error_to_string (-retval));
    }
    else if (vaddr == PT_START (vaddr))
    {
      ++curr_pt;

      if ((retval = atomik_pd_map_pagetable (pd_cap, curr_pt, vaddr)) < 0)
        ELF32_THROW ("Map table failed (stack): %s", error_to_string (-retval));
    }

    if ((retval = atomik_pt_map_page (
                      curr_pt,
                      &page_caps[pages + i],
                      vaddr)) < 0)
      ELF32_THROW ("Map page failed (stack): %s", error_to_string (-retval));

    if ((retval = atomik_page_remap (
                    &page_caps[pages + i],
                    ATOMIK_ACCESS_READ | ATOMIK_ACCESS_WRITE)) < 0)
      ELF32_THROW (
          "Adjust page attributes failed: %s",
          error_to_string (-retval));
  }

  ELF32_MSG ("Configuring TCB...");

  /* Configure TCB */
  if ((retval = atomik_tcb_configure (
                    tcb_cap,
                    ep_cap,
                    0,
                    croot,
                    pd_cap,
                    &page_caps[pages + ELF32_TASK_TCB_STACK_PAGES - 1])) < 0)
    ELF32_THROW ("Configure root task TCB: %s", error_to_string (-retval));

  return tcb_cap;

fail:
  /* TODO: Cleanup */

  return NULL;
}

