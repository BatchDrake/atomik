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

#include <stdlib.h>
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

  for (i = 0; i < count; ++i)
    if (env->root->cnode.base[i].object_type == ATOMIK_OBJTYPE_NULL)
      break;

  ATOMIK_TEST_ASSERT (i + 4 <= count);

  destination = &env->root->cnode.base[i];

  for (i = 0; i < 4; ++i)
    ATOMIK_TEST_ASSERT (destination[i].object_type == ATOMIK_OBJTYPE_NULL);

  if ((exception = atomik_untyped_retype (ut,
                                          ATOMIK_OBJTYPE_UNTYPED,
                                          5,
                                          destination,
                                          4)) != ATOMIK_SUCCESS)
  {
    debug (env, "call to retype failed: error %d\n", exception);

    ATOMIK_FAIL (ATOMIK_ERROR_TEST_FAILED);
  }

  off = i;

  ATOMIK_TEST_ASSERT (ut->mdb_child == &destination[0]);

  for (i = 0; i < 4; ++i)
  {
    ATOMIK_TEST_ASSERT (destination[i].object_type == ATOMIK_OBJTYPE_UNTYPED);
    ATOMIK_TEST_ASSERT (destination[i].mdb_parent  == ut);

    if (i > 0)
      ATOMIK_TEST_ASSERT (destination[i].mdb_prev == &destination[i - 1])
    else
      ATOMIK_TEST_ASSERT (destination[i].mdb_prev == NULL)

    if (i < 3)
      ATOMIK_TEST_ASSERT (destination[i].mdb_next == &destination[i + 1])
    else
      ATOMIK_TEST_ASSERT (destination[i].mdb_next == NULL)
  }

fail:
  /* Delete all derived capabilities */
  if (ut != NULL)
    atomik_capslot_revoke (ut);

  return exception;
}

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

  ut = test_find_small_ut (env->root, ATOMIK_PAGE_SIZE_BITS);

  ATOMIK_TEST_ASSERT (ut != NULL);

  for (i = 0; i < count; ++i)
    if (env->root->cnode.base[i].object_type == ATOMIK_OBJTYPE_NULL)
      break;

  ATOMIK_TEST_ASSERT (i < count);

  destination = &env->root->cnode.base[i];

  if ((exception = atomik_untyped_retype (ut,
                                          ATOMIK_OBJTYPE_PD,
                                          ATOMIK_PAGE_SIZE_BITS,
                                          destination,
                                          1)) != ATOMIK_SUCCESS)
  {
    debug (env, "call to retype failed: error %d\n", exception);

    ATOMIK_FAIL (ATOMIK_ERROR_TEST_FAILED);
  }

  kernel_size = __arch_get_kernel_layout ((void **) &kernel_virt_start, &kernel_phys_start);

  for (i = 0; i < kernel_size; i += PAGE_SIZE)
  {
    phys = capslot_vspace_resolve (destination, kernel_virt_start + i, 0, &exception);

    if (phys == ATOMIK_INVALID_ADDR)
    {
      debug (env, "unexpected error resolving address %p: error %d\n",
             kernel_virt_start + i,
             exception);
      goto fail;
    }

    if (phys != kernel_phys_start + i)
    {
      debug (env, "translation error: expected %p, got %p instead\n",
                   kernel_phys_start + i,
                   phys,
                   exception);
            goto fail;
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
  /* TODO: Check whether paging works (i.e. translate) */

  return ATOMIK_SUCCESS;
}

static error_t
test_vspace_switch (struct atomik_test_env *env)
{
  /* TODO: Check whether we can switch to a given vspace */

  return ATOMIK_SUCCESS;
}

static error_t
test_cdt_ops (struct atomik_test_env *env)
{
  /* TODO: Check whether CDT operations work (delete, mint, revoke) */

  return ATOMIK_SUCCESS;
}

struct atomik_test test_list[] =
    {
        {"ut_coverage",   test_ut_coverage},
        {"ut_retype",     test_ut_retype},
        {"vspace",        test_vspace},
        {"vspace_paging", test_vspace_paging},
        {"vspace_switch", test_vspace_switch},
        {"cdt_ops",       test_cdt_ops},
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
