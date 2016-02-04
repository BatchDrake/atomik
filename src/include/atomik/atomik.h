/*
 *    atomik.h: Macros and definitions required by all Atomik sources
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

#ifndef _ATOMIK_H
#define _ATOMIK_H

#include <alltypes.h>
#include <machinedefs.h>
#include <util.h>


#define ATOMIK_FAIL(code) \
{ \
  exception = code; \
  goto fail; \
}

enum error
{
  ATOMIK_SUCCESS,
  ATOMIK_ERROR_INVALID_ARGUMENT,
  ATOMIK_ERROR_INVALID_SIZE,
  ATOMIK_ERROR_INVALID_TYPE,
  ATOMIK_ERROR_INVALID_CAPABILITY,
  ATOMIK_ERROR_ILLEGAL_OPERATION,
  ATOMIK_ERROR_RANGE,
  ATOMIK_ERROR_FAILED_LOOKUP,
  ATOMIK_ERROR_DELETE_FIRST,
  ATOMIK_ERROR_REVOKE_FIRST,
  ATOMIK_ERROR_NOT_ENOUGH_MEMORY,
  ATOMIK_ERROR_PAGES_ONLY,
  ATOMIK_ERROR_TEST_FAILED
};

typedef enum error error_t;

#endif /* _ATOMIK_H */
