/*
 *    i386-seg.h: Segment definitions for i386
 *    Copyright (C) 2016 Gonzalo J. Carracedo
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
    
#ifndef _ARCH_I386_SEG_H
#define _ARCH_I386_SEG_H

#include <util.h>
#include <alltypes.h>

#define GDT_SEGMENT_KERNEL_CODE 0x8
#define GDT_SEGMENT_KERNEL_DATA 0x10
#define GDT_SEGMENT_USER_CODE   0x18
#define GDT_SEGMENT_USER_DATA   0x20
#define GDT_SEGMENT_TSS         0x28
#define GDT_SEGMENT_USER_TLS    0x30

#define GDT_ENTRY(x) (&gdt[(x) >> 3])

#define GDT_MAX_SEGMENTS        7

#define GDT_GRANULARITY_PAGES   0x08
#define GDT_SELECTOR_32_BIT     0x04

#define GDT_ACCESS_ACCESED      0x01
#define GDT_ACCESS_READWRITE    0x02
#define GDT_ACCESS_GROW_DOWN    0x04
#define GDT_ACCESS_CONFORMING   0x04 /* Far jumps from lower privs allowed */
#define GDT_ACCESS_EXECUTABLE   0x08
#define GDT_ACCESS_SEGMENT      0x10
#define GDT_ACCESS_RING(x)      ((x) << 5)
#define GDT_ACCESS_PRESENT      0x80

struct tss
{
  uint16_t  link;
  uint16_t  reserved_1;
  uint32_t esp0;
  uint16_t  ss0;
  uint16_t  reserved_2; 
  uint32_t esp1;
  uint16_t  ss1;
  uint16_t  reserved_3;
  uint32_t esp2;
  uint16_t  ss2;
  uint16_t  reserved_4;
  
  uint32_t pagedir;
  
  uint32_t eip, eflags;
  
  uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
  
  uint16_t  es, reserv_es;
  uint16_t  cs, reserv_cs;
  uint16_t  ss, reserv_ss;
  uint16_t  ds, reserv_ds;
  uint16_t  fs, reserv_fs;
  uint16_t  gs, reserv_gs;
  uint16_t  ldtr, reserv_ldtr;
  
  uint16_t  reserv_iopb;
  
  uint16_t  iopb; 
} PACKED;

struct gdt_entry
{
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t limit_high:4;
  uint8_t flags:4;
  uint8_t base_high;
} PACKED;

struct gdt_ptr
{
  uint16_t limit;
  struct gdt_entry *base;
} PACKED;

void i386_seg_init (void);
void i386_get_current_gdt (struct gdt_ptr *);
void i386_flush_gdt (struct gdt_ptr *);
void i386_flush_tss (void);
void i386_set_kernel_stack (uint32_t);
void i386_enter_user (void);

#endif /* _ARCH_I386_SEG_H */
