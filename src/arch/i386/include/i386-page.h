/*
 *    i386-page.h: Some useful constants for x86 paging
 *    Copyright (C) 2014  Gonzalo J. Carracedo
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

#ifndef _ARCH_I386_PAGE_H
#define _ARCH_I386_PAGE_H

#define PAGE_FLAG_PRESENT       1
#define PAGE_FLAG_WRITABLE      2 /* see cr0's WP bit for details */
#define PAGE_FLAG_USERLAND      4
#define PAGE_FLAG_WRITE_THROUGH 8
#define PAGE_FLAG_CACHE_DISABLE 16
#define PAGE_FLAG_ACCESSED      32
#define PAGE_FLAG_DIRTY         64
#define PAGE_FLAG_4MIB_PAGES    128
#define PAGE_FLAG_GLOBAL        256

#define PAGE_TABLE_DFL_FLAGS    (PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE)

#endif /* _ARCH_I386_PAGE_H */
