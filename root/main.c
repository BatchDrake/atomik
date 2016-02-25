/*
 *    main.c: Entry point for the root task
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
#include <atomik-user.h>

char *text = "Hello";

void
d_puts (const char *msg)
{
  while (*msg)
    d_putch (*msg++);
}

void
_start (void)
{
  d_puts ("Hello world! This is my first userland program!\n");

  d_halt ();
}
