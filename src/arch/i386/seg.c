/*
 *    seg.c: Global descriptor table setup
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

#include <atomik/atomik.h>

#include <i386-layout.h>
#include <i386-seg.h>

struct gdt_entry gdt[GDT_MAX_SEGMENTS];

/* Defined in boot.c, this variable is remapped */
extern struct tss tss;

static void
gdt_entry_setup (struct gdt_entry *dest,
                  uint32_t base, uint32_t page_limit, uint8_t access)
{  
  dest->limit_low  = page_limit & 0xffff;
  dest->base_low   = base & 0xffff;
  dest->base_mid   = (base >> 16) & 0xff;
  dest->base_high  = (base >> 24) & 0xff;
  dest->limit_high = (page_limit >> 16) & 0xf;
  dest->access     = access;
  dest->flags      = GDT_GRANULARITY_PAGES | GDT_SELECTOR_32_BIT;
}

static void
gdt_entry_setup_tss (struct gdt_entry *dest)
{
  uint32_t tss_base  = (uint32_t) &tss;
  uint32_t tss_limit = tss_base + sizeof (struct tss) - 1;

  /* Kernel stack is in kernel data segment */
  
  tss.ss0          = GDT_SEGMENT_KERNEL_DATA;
  
  dest->limit_low  = tss_limit         & 0xffff;
  dest->base_low   = tss_base          & 0xffff;
  dest->base_mid   = (tss_base  >> 16) & 0xff;
  dest->base_high  = (tss_base  >> 24) & 0xff;
  dest->limit_high = (tss_limit >> 16) & 0xf;
  
  dest->access     = GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_ACCESED | GDT_ACCESS_RING (3);
  dest->flags      = GDT_SELECTOR_32_BIT;
}

#if 0
void
i386_setup_tls (uint32_t base, uint32_t limit)
{
  /* Please note that USER_TLS_BASE is in the middle of a page */
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_USER_TLS), base, limit, 
    GDT_ACCESS_READWRITE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT |
    GDT_ACCESS_RING (3));
}
#endif

void
i386_set_kernel_stack (uint32_t addr)
{
  /* Setting the kernel stack has to be done through this address */
  tss.esp0 = addr;
}

void
i386_seg_init (void)
{
  struct gdt_ptr ptr, ptr2;
  uint8_t *gdt_bytes;
  int i;
  
  ptr.limit = sizeof (struct gdt_entry) * GDT_MAX_SEGMENTS - 1;
  ptr.base  = gdt;

  gdt_bytes = (uint8_t *) gdt;
  
  for (i = 0; i <= ptr.limit; ++i)
    gdt_bytes[i] = 0;
  
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_KERNEL_CODE), 0, 0xfffff,
    GDT_ACCESS_EXECUTABLE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT);
    
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_KERNEL_DATA), 0, 0xfffff,
    GDT_ACCESS_READWRITE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT);
  
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_USER_CODE), 0, 0xfffff,
    GDT_ACCESS_EXECUTABLE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT |
    GDT_ACCESS_RING (3));
    
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_USER_DATA), 0, 0xfffff, 
    GDT_ACCESS_READWRITE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT |
    GDT_ACCESS_RING (3));
  
  gdt_entry_setup_tss (GDT_ENTRY (GDT_SEGMENT_TSS));

  i386_flush_gdt (&ptr);
  
  i386_flush_tss ();
  
  i386_get_current_gdt (&ptr2);
}

