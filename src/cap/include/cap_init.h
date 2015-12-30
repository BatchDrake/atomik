/*
 *    cap_init.h: Definitions for the initial set of capabilities
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

#ifndef _CAP_INIT_H
#define _CAP_INIT_H

#include <machinedefs.h>
#include <atomik/cap.h>

#define ATOMIK_INIT_CAP_CNODE_SIZE (PAGE_BITS - ATOMIK_CAPSLOT_SIZE_BITS)
#define ATOMIK_INIT_CAP_GUARD_BITS 4
#define ATOMIK_INIT_CAP_GUARD      0xa



#endif /* _CAP_INIT_H */
