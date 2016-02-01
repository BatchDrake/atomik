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

  debug (env, "Counting UT capabilities (should be %d bytes...)\n", free_size);

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

  debug (env, "Testing UT retyping\n");

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
    debug (env, "Call to retype failed: error %d\n", exception);

    ATOMIK_FAIL (ATOMIK_ERROR_TEST_FAILED);
  }

  off = i;

  ATOMIK_TEST_ASSERT (ut->mdb_child == &destination[0]);

  for (i = 0; i < 4; ++i)
  {
    ATOMIK_TEST_ASSERT (destination[i].object_type == ATOMIK_OBJTYPE_UNTYPED);
    ATOMIK_TEST_ASSERT (destination[i].mdb_parent  == ut);

    if (i > 0)
    {
      ATOMIK_TEST_ASSERT (destination[i].mdb_prev == &destination[i - 1]);
    }
    else
    {
      ATOMIK_TEST_ASSERT (destination[i].mdb_prev == NULL);
    }

    if (i < 3)
    {
      ATOMIK_TEST_ASSERT (destination[i].mdb_next == &destination[i + 1]);
    }
    else
    {
      ATOMIK_TEST_ASSERT (destination[i].mdb_next == NULL);
    }
  }

fail:
  if (destination != NULL && off != -1)
    for (i = 0; i < 4; ++i)
    {
      /* Delete */
    }

  return exception;
}

struct atomik_test test_list[] =
    {
        {"ut_coverage", test_ut_coverage},
        {"ut_retype",   test_ut_retype},
        {NULL, NULL}
    };

int
run_tests (capslot_t *root)
{
  int i = 0;
  struct atomik_test_env env;

  env.root = root;

  while (test_list[i].test_func != NULL)
  {
    env.test = &test_list[i];

    debug (&env, "Test starting...\n");
    if (test_list[i].test_func (&env) != ATOMIK_SUCCESS)
    {
      debug (&env, "ERROR: Test failed\n");
      return -ATOMIK_ERROR_TEST_FAILED;
    }

    debug (&env, "OK!\n");

    ++i;
  }

  return ATOMIK_SUCCESS;
}
