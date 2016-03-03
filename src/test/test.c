/*
 *    test.c: General unit tests
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
#include <atomik/test.h>
#include <atomik/vspace.h>

#include <arch.h>

#include <test.h>

static error_t
test_ut_coverage (struct atomik_test_env *env)
{
  unsigned int count;
  unsigned int i;
  size_t total = 0;

  void  *free_start;
  size_t free_size;

  void  *remap_start;
  size_t remap_size;

  __arch_get_free_memory (&free_start, &free_size);
  __arch_get_kernel_remap (&remap_start, &remap_size);

  /* Note: substract 1 page from root cnode */

  free_size -= PAGE_SIZE;

  debug (env, "counting UT capabilities (should be %d bytes...)\n", free_size);

  count = CNODE_COUNT (env->root);

  for (i = 0; i < count; ++i)
    if (env->root->cnode.base[i].object_type == ATOMIK_OBJTYPE_UNTYPED)
      total += UT_SIZE (&env->root->cnode.base[i]);

  if (total != free_size)
  {
    debug (env, "UT count mismatch: expected %d, got %d\n", free_size, total);
    return ATOMIK_ERROR_TEST_FAILED;
  }

  return ATOMIK_SUCCESS;
}

/* TODO: Force lookup of remappable UT */
static capslot_t *
test_find_small_ut (const capslot_t *root, unsigned int bits)
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
test_find_free_capslots (const capslot_t *root, unsigned int count)
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

static error_t
test_ut_retype (struct atomik_test_env *env)
{
  unsigned int count;
  unsigned int i;
  int off = -1;
  capslot_t *ut;
  capslot_t *destination;

  error_t exception = ATOMIK_SUCCESS;

  count = CNODE_COUNT (env->root);

  ut = test_find_small_ut (env->root, 7);
  ATOMIK_TEST_ASSERT (ut != NULL);

  destination = test_find_free_capslots (env->root, 4);
  ATOMIK_TEST_ASSERT (destination != NULL);

  ATOMIK_TEST_ASSERT_SUCCESS (
      atomik_untyped_retype (ut, ATOMIK_OBJTYPE_UNTYPED, 5, destination, 4));

  off = i;

  ATOMIK_TEST_ASSERT (ut->mdb_child == &destination[3]);

  for (i = 0; i < 4; ++i)
  {
    ATOMIK_TEST_ASSERT (destination[i].object_type == ATOMIK_OBJTYPE_UNTYPED);
    ATOMIK_TEST_ASSERT (destination[i].mdb_parent  == ut);

    if (i < 3)
      ATOMIK_TEST_ASSERT (destination[i].mdb_prev == &destination[i + 1])
    else
      ATOMIK_TEST_ASSERT (destination[i].mdb_prev == NULL)

    if (i > 0)
      ATOMIK_TEST_ASSERT (destination[i].mdb_next == &destination[i - 1])
    else
      ATOMIK_TEST_ASSERT (destination[i].mdb_next == NULL)
  }

fail:
  /* Delete all derived capabilities */
  if (ut != NULL)
    atomik_capslot_revoke (ut);

  return exception;
}

extern unsigned int tss;

#include <i386-seg.h>
static error_t
test_vspace (struct atomik_test_env *env)
{
  int off = -1;
  unsigned int i, count;
  capslot_t *ut;
  capslot_t *destination;
  error_t exception = ATOMIK_SUCCESS;
  uintptr_t phys;
  size_t kernel_size;
  uintptr_t kernel_virt_start;
  uintptr_t kernel_phys_start;

  count = CNODE_COUNT (env->root);

  ut = test_find_small_ut (env->root, ATOMIK_PD_SIZE_BITS);
  ATOMIK_TEST_ASSERT (ut != NULL);

  destination = test_find_free_capslots (env->root, 1);
  ATOMIK_TEST_ASSERT (destination != NULL);

  ATOMIK_TEST_ASSERT_SUCCESS (
      -atomik_untyped_retype (
        ut,
        ATOMIK_OBJTYPE_PD,
        ATOMIK_PD_SIZE_BITS,
        destination,
        1));

  kernel_size = __arch_get_kernel_layout ((void **) &kernel_virt_start, &kernel_phys_start);

  for (i = 0; i < kernel_size; i += PAGE_SIZE)
  {
    if (kernel_virt_start + i != (uintptr_t) &tss)
    {
      phys = capslot_vspace_resolve (destination, kernel_virt_start + i, 0, &exception);

      ATOMIK_TEST_ASSERT (phys != ATOMIK_INVALID_ADDR);

      if (phys != kernel_phys_start + i)
      {
        debug (env, "translation error: expected %p, got %p instead\n",
                     kernel_phys_start + i,
                     phys);

        ATOMIK_FAIL (ATOMIK_ERROR_TEST_FAILED);
      }
    }
  }

fail:
  /* Delete all derived capabilities */
  if (ut != NULL)
    atomik_capslot_revoke (ut);

  return exception;
}

static error_t
test_vspace_paging (struct atomik_test_env *env)
{
  unsigned int i, count;
  capslot_t *ut;
  capslot_t *destination;
  capslot_t *pd, *pt, *pages;
  error_t exception = ATOMIK_SUCCESS;
  int should_read, should_write, should_execute;
  int could_read, could_write, could_execute;
  uintptr_t phys;

  count = CNODE_COUNT (env->root);

  /* We require
   * a) 1 PD
   * b) 1 PT
   * c) 8 pages
   *
   * This is 2^4 pages of bytes.
   * We also require 10 UT slots and 10 slots for conversion.
   */
  ut = test_find_small_ut (env->root, ATOMIK_PAGE_SIZE_BITS + 4);
  ATOMIK_TEST_ASSERT (ut != NULL);

  /* This is where all UTs will be placed */
  destination = test_find_free_capslots (env->root, 10);
  ATOMIK_TEST_ASSERT (destination != NULL);

  /* Split UT into smaller UTs */
  ATOMIK_TEST_ASSERT_SUCCESS (
      -atomik_untyped_retype (ut, ATOMIK_OBJTYPE_UNTYPED, ATOMIK_PAGE_SIZE_BITS,
                              destination,
                              10));


  /* Initialize, PD, PT and pages */
  pd    = test_find_free_capslots (env->root, 1);
  ATOMIK_TEST_ASSERT (pd != NULL);
  ATOMIK_TEST_ASSERT_SUCCESS (
      -atomik_untyped_retype (&destination[0], ATOMIK_OBJTYPE_PD,
                              ATOMIK_PD_SIZE_BITS,
                              pd,
                              1));

  pt    = test_find_free_capslots (env->root, 1);
  ATOMIK_TEST_ASSERT (pt != NULL);
  ATOMIK_TEST_ASSERT_SUCCESS (
      -atomik_untyped_retype (&destination[1], ATOMIK_OBJTYPE_PT,
                              ATOMIK_PT_SIZE_BITS,
                              pt,
                              1));

  pages = test_find_free_capslots (env->root, 8);
  ATOMIK_TEST_ASSERT (pages != NULL);
  for (i = 0; i < 8; ++i)
    ATOMIK_TEST_ASSERT_SUCCESS (
        -atomik_untyped_retype (&destination[2 + i], ATOMIK_OBJTYPE_PAGE,
                                ATOMIK_PAGE_SIZE_BITS,
                                &pages[i],
                                1)); /* 8 pages */

  ATOMIK_TEST_ASSERT_SUCCESS (-atomik_pd_map_pagetable (pd, pt, 0x08000000));

  for (i = 0; i < 8; ++i)
  {
    ATOMIK_TEST_ASSERT_SUCCESS (
        -atomik_pt_map_page (pt, &pages[i], 0x08048000 + PAGE_ADDRESS (i)));

    ATOMIK_TEST_ASSERT (
        __atomik_capslot_get_page_vaddr (&pages[i]) ==
            0x08048000 + PAGE_ADDRESS (i));

    /* Drop privileges */
    ATOMIK_TEST_ASSERT_SUCCESS (-atomik_capslot_drop (&pages[i], ~i));
  }

  /* Phase 1: Test whether all virtual addresses have been correctly mapped */
  debug (env, "Phase 1: Resolving all mapped addresses...\n");

  for (i = 0; i < 8; ++i)
  {
    debug (env, "  0x%08x? ", 0x08048000 + PAGE_ADDRESS (i));

    phys = capslot_vspace_resolve (pd,
                                   0x08048000 + PAGE_ADDRESS (i),
                                   0,
                                   &exception);

    printf ("0x%08x\n", phys);

    if (phys == ATOMIK_INVALID_ADDR)
    {
      debug (env, "Translation error: cannot translate page #%d\n", i);
      debug (env, "capslot_vspace_resolve failed (%s)\n",
             error_to_string (exception));

      goto fail;
    }

    if (phys != (uintptr_t) pages[i].page.base)
    {
      debug (env, "Unexpected physical address. Expected %p, got %p\n",
             pages[i].page.base,
             phys);
      ATOMIK_FAIL (ATOMIK_ERROR_TEST_FAILED);
    }
  }

  /* Phase 2: Test access */
  debug (env, "Phase 2: Trying different access modes...\n");

  for (i = 0; i < 8; ++i)
  {
    should_read    = !!(i & ATOMIK_ACCESS_READ);
    should_write   = !!(i & ATOMIK_ACCESS_WRITE);
    should_execute = !!(i & ATOMIK_ACCESS_EXEC);

    debug (env, "  0x%08x (%c%c%c)? ",
           0x08048000 + PAGE_ADDRESS (i),
           should_read ? 'r' : '-',
           should_write ? 'w' : '-',
           should_execute ? 'x' : '-');

    phys = capslot_vspace_resolve (pd, 0x08048000 + PAGE_ADDRESS (i),
                                   ATOMIK_ACCESS_READ,
                                   &exception);
    ATOMIK_TEST_ASSERT (exception == ATOMIK_SUCCESS ||
                        exception == ATOMIK_ERROR_ACCESS_DENIED);
    could_read = phys != ATOMIK_INVALID_ADDR;

    phys = capslot_vspace_resolve (pd, 0x08048000 + PAGE_ADDRESS (i),
                                   ATOMIK_ACCESS_WRITE,
                                   &exception);
    ATOMIK_TEST_ASSERT (exception == ATOMIK_SUCCESS ||
                        exception == ATOMIK_ERROR_ACCESS_DENIED);
    could_write = phys != ATOMIK_INVALID_ADDR;

    phys = capslot_vspace_resolve (pd, 0x08048000 + PAGE_ADDRESS (i),
                                   ATOMIK_ACCESS_EXEC,
                                   &exception);
    ATOMIK_TEST_ASSERT (exception == ATOMIK_SUCCESS ||
                        exception == ATOMIK_ERROR_ACCESS_DENIED);
    could_execute = phys != ATOMIK_INVALID_ADDR;

    printf ("%c%c%c\n",
           could_read ? 'r' : '-',
           could_write ? 'w' : '-',
           could_execute ? 'x' : '-');

    /* In x86 there are some access configurations that are not possible */

    if (should_read)
      ATOMIK_TEST_ASSERT (could_read == should_read);

    ATOMIK_TEST_ASSERT (could_write == should_write);

    if (should_execute)
      ATOMIK_TEST_ASSERT (could_execute == should_execute);
  }

  exception = ATOMIK_SUCCESS;

fail:
  /* Delete all derived capabilities */
  if (ut != NULL)
    atomik_capslot_revoke (ut);

  return exception;
}

static error_t
test_vspace_switch (struct atomik_test_env *env)
{
  unsigned int i, count;
  capslot_t *ut;
  capslot_t *destination;
  capslot_t *pd, *pt, *pages;
  error_t exception = ATOMIK_SUCCESS;
  uintptr_t phys;
  char *remapped, *virt;

  count = CNODE_COUNT (env->root);

  ut = test_find_small_ut (env->root, ATOMIK_PAGE_SIZE_BITS + 4);
  ATOMIK_TEST_ASSERT (ut != NULL);

  /* This is where all UTs will be placed */
  destination = test_find_free_capslots (env->root, 10);
  ATOMIK_TEST_ASSERT (destination != NULL);

  /* Split UT into smaller UTs */
  ATOMIK_TEST_ASSERT_SUCCESS (
      -atomik_untyped_retype (ut, ATOMIK_OBJTYPE_UNTYPED, ATOMIK_PAGE_SIZE_BITS,
                              destination,
                              10));


  /* Initialize, PD, PT and pages */
  pd    = test_find_free_capslots (env->root, 1);
  ATOMIK_TEST_ASSERT (pd != NULL);
  ATOMIK_TEST_ASSERT_SUCCESS (
      -atomik_untyped_retype (&destination[0], ATOMIK_OBJTYPE_PD,
                              ATOMIK_PD_SIZE_BITS,
                              pd,
                              1));

  pt    = test_find_free_capslots (env->root, 1);
  ATOMIK_TEST_ASSERT (pt != NULL);
  ATOMIK_TEST_ASSERT_SUCCESS (
      -atomik_untyped_retype (&destination[1], ATOMIK_OBJTYPE_PT,
                              ATOMIK_PT_SIZE_BITS,
                              pt,
                              1));

  pages = test_find_free_capslots (env->root, 8);
  ATOMIK_TEST_ASSERT (pages != NULL);
  for (i = 0; i < 8; ++i)
    ATOMIK_TEST_ASSERT_SUCCESS (
        -atomik_untyped_retype (&destination[2 + i], ATOMIK_OBJTYPE_PAGE,
                                ATOMIK_PAGE_SIZE_BITS,
                                &pages[i],
                                1)); /* 8 pages */

  ATOMIK_TEST_ASSERT_SUCCESS (-atomik_pd_map_pagetable (pd, pt, 0x08000000));

  /* Map all eight pages */
  for (i = 0; i < 8; ++i)
  {
    ATOMIK_TEST_ASSERT_SUCCESS (
        -atomik_pt_map_page (pt, &pages[i], 0x08048000 + PAGE_ADDRESS (i)));

    /* TODO: this is dangerous */
    sprintf (__atomik_phys_to_remap ((uintptr_t) pages[i].page.base),
             "This is page %d (physical address %p)",
             i,
             pages[i].page.base);
  }

  debug (env, "Switching to new vspace...\n");

  /* Switch vspace. This is one of the most critical operations we
   * can perform in kernel mode. */
  ATOMIK_TEST_ASSERT_SUCCESS (-capslot_vspace_switch (pd));

  debug (env, "Address space switched. Page contents:\n");

  for (i = 0; i < 8; ++i)
  {
    remapped = (char *) __atomik_phys_to_remap ((uintptr_t) pages[i].page.base);
    virt     = (char *) __atomik_capslot_get_page_vaddr (&pages[i]);

    debug (env, "  %p -> \"%s\"\n", virt, virt);

    ATOMIK_TEST_ASSERT (strcmp (remapped, virt) == 0);
  }

  /* Come back */
  ATOMIK_TEST_ASSERT_SUCCESS (-capslot_vspace_switch (NULL));

  exception = ATOMIK_SUCCESS;

fail:
  /* Delete all derived capabilities */
  if (ut != NULL)
    atomik_capslot_revoke (ut);

  return exception;
}

static error_t
test_cdt_ops (struct atomik_test_env *env)
{
  /* TODO: Check whether CDT operations work (delete, mint, revoke) */

  return ATOMIK_SUCCESS;
}

static error_t
test_pool_ops (struct atomik_test_env *env)
{
  error_t exception = ATOMIK_SUCCESS;
  capslot_t *pool_ut = NULL;
  capslot_t *cnode_ut = NULL;
  capslot_t pool = capslot_t_INITIALIZER;
  capslot_t cnode = capslot_t_INITIALIZER;

  capslot_t *page, *prev;
  void *prev_base;
  unsigned int i;
  int err;
  
  /* I am going to allocate a pool big enough */
  pool_ut = test_find_small_ut (
    env->root,
    ATOMIK_PAGE_SIZE_BITS + 12);
  
  ATOMIK_TEST_ASSERT (pool_ut != NULL);

  /* Allocate pool */
  ATOMIK_TEST_ASSERT_SUCCESS (
      -atomik_untyped_retype (
        pool_ut,
        ATOMIK_OBJTYPE_POOL,
        ATOMIK_PAGE_SIZE_BITS + 12,
        &pool,
        1));

  /* Allocate CNode of 4096 entries */
  cnode_ut = test_find_small_ut (
    env->root,
    ATOMIK_CAPSLOT_SIZE_BITS + 12);

  ATOMIK_TEST_ASSERT (cnode_ut != NULL);

  ATOMIK_TEST_ASSERT_SUCCESS (
    -atomik_untyped_retype (
      cnode_ut,
      ATOMIK_OBJTYPE_CNODE,
      12, /* Cnode of 12 entries */
      &cnode,
      1));

  debug (env, "Objects retyped, testing use cases...\n");
  
  /* We shouldn't be able to allocate objects at
     this point */
  err = atomik_pool_alloc (&pool, CNODE_BASE (&cnode));

  ATOMIK_ASSERT (err != -ATOMIK_SUCCESS);
  ATOMIK_ASSERT (err == -ATOMIK_ERROR_INIT_FIRST);

  debug (env, "Retyping pool...\n");
  
  /* This should work: pool isn't retyped yet  */
  ATOMIK_TEST_ASSERT_SUCCESS (
    -atomik_pool_retype (&pool, ATOMIK_OBJTYPE_PAGE, 0));

  debug (env, "Testing double retype (it should fail)...\n");
  
  /* This shouldn't work: we cannot retype a pool twice */
  err = atomik_pool_retype (&pool, ATOMIK_OBJTYPE_PAGE, 0);

  ATOMIK_ASSERT (err != -ATOMIK_SUCCESS);
  ATOMIK_ASSERT (err == -ATOMIK_ERROR_DELETE_FIRST);

  debug (env, "Allocating 3000 pages...\n");
  
  /* Start to allocate pages */
  for (i = 0; i < 3000; ++i)
    ATOMIK_TEST_ASSERT_SUCCESS (
      atomik_pool_alloc (&pool, CNODE_BASE (&cnode) + i));

  prev_base = PAGE_BASE (CNODE_BASE (&cnode) + 1500);
  debug (env, "Done, cap 1500 @ %p\n", prev_base);
  
  /* Delete one */
  ATOMIK_TEST_ASSERT_SUCCESS (
    -atomik_capslot_delete (CNODE_BASE (&cnode) + 1500));

  /* Reallocate */
  ATOMIK_TEST_ASSERT_SUCCESS (
    -atomik_pool_alloc (&pool, CNODE_BASE (&cnode) + 1500));
    
  /* Check whether we've allocated it in the previous
     address */
  ATOMIK_TEST_ASSERT (
    PAGE_BASE (CNODE_BASE (&cnode) + 1500) == prev_base);

  /* Revoke everything */
  ATOMIK_TEST_ASSERT_SUCCESS (
    -atomik_capslot_revoke (pool_ut));

  pool_ut = NULL;
  
  ATOMIK_TEST_ASSERT_SUCCESS (
    -atomik_capslot_revoke (cnode_ut));

  cnode_ut = NULL;

  ATOMIK_TEST_ASSERT (pool.object_type  == ATOMIK_OBJTYPE_NULL);

  ATOMIK_TEST_ASSERT (cnode.object_type == ATOMIK_OBJTYPE_NULL);
  
fail:
  if (pool_ut != NULL)
    (void) atomik_capslot_revoke (pool_ut);

  if (cnode_ut != NULL)
    (void) atomik_capslot_revoke (cnode_ut);
  
  return exception;
}

struct atomik_test test_list[] =
    {
        {"ut_coverage",   test_ut_coverage},
        {"ut_retype",     test_ut_retype},
        {"vspace",        test_vspace},
        {"vspace_paging", test_vspace_paging},
        {"vspace_switch", test_vspace_switch},
        {"cdt_ops",       test_cdt_ops},
        {"test_pool_ops", test_pool_ops},
        {NULL, NULL}
    };

int
run_tests (capslot_t *root)
{
  struct atomik_test_env env;

  env.root  = root;
  env.total = 0;
  env.ok    = 0;

  while (test_list[env.total].test_func != NULL)
  {
    env.test = &test_list[env.total++];

    debug (&env, "test starting...\n");
    if (env.test->test_func (&env) != ATOMIK_SUCCESS)
      debug (&env, "\033[1;31merror\033[0m: test failed\n");
    else
    {
      debug (&env, "\033[1;32mok\033[0m!\n");
      ++env.ok;
    }
  }

  printf ("ATOMIK TEST RESULTS: \n");
  printf ("  Number of tests run: %d\n", env.total);
  printf ("  Number of failures:  %d\n", env.total - env.ok);

  if (env.total != env.ok)
    return ATOMIK_ERROR_TEST_FAILED;

  return ATOMIK_SUCCESS;
}
