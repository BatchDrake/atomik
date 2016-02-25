/*
 *    util.h: Some useful utility functions
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

#include <alltypes.h>
#include <util.h>

const char *
error_to_string (error_t err)
{
  switch (err)
  {
    case ATOMIK_SUCCESS:
      return "No errors";

    case ATOMIK_ERROR_INVALID_ARGUMENT:
      return "Invalid argument for function";

    case ATOMIK_ERROR_INVALID_SIZE:
      return "Invalid size for object";

    case ATOMIK_ERROR_INVALID_TYPE:
      return "Invalid type for object";

    case ATOMIK_ERROR_INVALID_CAPABILITY:
      return "Invalid capability type for function";

    case ATOMIK_ERROR_ILLEGAL_OPERATION:
      return "Illegal operation on object";

    case ATOMIK_ERROR_ACCESS_DENIED:
      return "Access denied";

    case ATOMIK_ERROR_INVALID_ADDRESS:
      return "Untranslatable address";

    case ATOMIK_ERROR_RANGE:
      return "Range error";

    case ATOMIK_ERROR_FAILED_LOOKUP:
      return "No such object";

    case ATOMIK_ERROR_DELETE_FIRST:
      return "Object must be deleted first";

    case ATOMIK_ERROR_REVOKE_FIRST:
      return "Derived objects must be deleted first";

    case ATOMIK_ERROR_MAP_FIRST:
      return "Object must be mapped first";

    case ATOMIK_ERROR_UNMAP_FIRST:
      return "Object must be unmapped first";

    case ATOMIK_ERROR_NOT_ENOUGH_MEMORY:
      return "No memory left in object";

    case ATOMIK_ERROR_PAGES_ONLY:
      return "Provided untyped object can only be retyped to page objects";

    case ATOMIK_ERROR_TEST_FAILED:
      return "Microkernel unit test has failed";

    case ATOMIK_ERROR_PULL_FIRST:
      return "Thread already in scheduler queues";

    case ATOMIK_ERROR_PUSH_FIRST:
      return "Thread is not in scheduler queues";

    case ATOMIK_ERROR_ALREADY_BOUND:
      return "Object is already bound to a TCB";

    default:
      return "Unknown error";
  }
}
