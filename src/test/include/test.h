/*
 *    test.h: In-kernel unit testing
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

#ifndef _TEST_TEST_H
#define _TEST_TEST_H

#include <stdio.h>

#include <util.h>

#include <atomik/atomik.h>
#include <atomik/cap.h>

#define debug(env, fmt, arg...) \
  printf ("[%d] %s: " fmt, (env)->total, (env)->test->test_name, ##arg)

#define ATOMIK_TEST_ASSERT(expr) \
{\
  if (!(expr)) \
  { \
    debug (env, "Test assertion failed: " STRINGIFY (expr) "\n"); \
    ATOMIK_FAIL (ATOMIK_ERROR_TEST_FAILED); \
  } \
}

#define ATOMIK_TEST_ASSERT_SUCCESS(expr) \
{\
  if ((exception = (expr)) != ATOMIK_SUCCESS) \
  { \
    debug (env, "Test assertion failed: " STRINGIFY (expr) \
           " (%s)\n", error_to_string (exception)); \
    goto fail; \
  } \
}

struct atomik_test_env
{
  struct atomik_test *test;
  capslot_t *root;
  unsigned int total;
  unsigned int ok;
};

struct atomik_test
{
  const char *test_name;
  error_t (*test_func) (struct atomik_test_env *);
};

#endif /* _TEST_TEST_H */
