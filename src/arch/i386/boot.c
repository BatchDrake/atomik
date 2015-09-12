/*
 *    multiboot.c: Multiboot handling functions
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

#include <atomik/atomik.h>

#include <stdlib.h> /* For NULL */

#include <i386-layout.h>
#include <i386-page.h>
#include <i386-regs.h>
#include <i386-vga.h>

#include <multiboot.h>

#define BOOTTIME_DEFAULT_ATTRIBUTE 0x1f

void main (void);

/* Symbols provided by linker */
extern int kernel_start;
extern int kernel_end;
extern int text_start;

/* Functions in case we need to output something REALLY REALLY early */
BOOT_FUNCTION (void boot_halt (void));
BOOT_FUNCTION (void boot_screen_clear (uint8_t));
BOOT_FUNCTION (void boot_puts (const char *));
BOOT_FUNCTION (void boot_print_hex (uint32_t));
BOOT_FUNCTION (void boot_print_dec (uint32_t));

BOOT_FUNCTION (void boot_entry (void));
BOOT_FUNCTION (void boot_fix_multiboot (void));

/* Symbols required by extern files */
BOOT_SYMBOL (uint32_t __free_start);
BOOT_SYMBOL (void *initrd_phys) = NULL;
BOOT_SYMBOL (void *initrd_start) = NULL;
BOOT_SYMBOL (uint32_t initrd_size) = 0;

BOOT_SYMBOL (char bootstack[4 * PAGE_SIZE]); /* Boot stack, as used by _start */
BOOT_SYMBOL (char cmdline_copy[128]);
BOOT_SYMBOL (struct multiboot_info *multiboot_info);

/* Inner state of boot_entry */
BOOT_SYMBOL (static int cur_x) = 0;
BOOT_SYMBOL (static int cur_y) = 0;
BOOT_SYMBOL (static struct page_table *page_dir);
BOOT_SYMBOL (static struct page_table *page_table_list);
BOOT_SYMBOL (static int page_table_count);

/* Some static text. It must be explicitly set up as boot_symbol, otherwise it would be linked inside .rodata (which is at upper half) */
BOOT_SYMBOL (static char string0[]) = "This is Atomik's boot_entry, v0.1 alpha\n(c) 2014 Gonzalo J. Carracedo <BatchDrake@gmail.com>\n\n";
  
BOOT_SYMBOL (static char string1[]) = "Moving to upper half, kernel from in 0x";
BOOT_SYMBOL (static char string2[]) = " to 0x";
BOOT_SYMBOL (static char string3[]) = " (";
BOOT_SYMBOL (static char string4[]) = " bytes)\n";
BOOT_SYMBOL (static char string5[]) = "Kernel virtual base address is 0x";
BOOT_SYMBOL (static char string6[]) = "\n\nConfiguring inital virtual address space...\n";
BOOT_SYMBOL (static char string7[]) = "\nMemory configuration done, switching to virtual memory and booting Atomik...\n";

/* Some static functions not needed outside */
BOOT_FUNCTION (static void boot_setup_vregion (uint32_t, uint32_t, uint32_t));
BOOT_FUNCTION (static void boot_outportb (uint16_t, uint8_t));
BOOT_FUNCTION (static void dword_to_decimal (uint32_t, char *));
BOOT_FUNCTION (static void dword_to_hex (uint32_t, char *));

BOOT_FUNCTION (static void boot_print_mmap (uint32_t, uint32_t, uint32_t));
BOOT_FUNCTION (static void boot_prepare_paging_early (void));


/* All functions here are boot-time */
void
got_multiboot (struct multiboot_info *mbi)
{
  multiboot_info = mbi;
}

struct multiboot_info *
multiboot_location (void)
{
  return multiboot_info;
}

const char *
kernel_command_line (void)
{
  if (multiboot_info->flags & (1 << 2))
    return (const char *) multiboot_info->cmdline;
  else
    return "";
}

static void
boot_outportb (uint16_t port, uint8_t data)
{
  __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}

void
boot_screen_clear (uint8_t attrib)
{
  schar *base = (schar *) VIDEO_BASE;
  int i, j;
  schar clear_char = {' ', attrib};
  uint16_t fullpos;
  
  for (j = 0; j < EGA_DEFAULT_SCREEN_HEIGHT; ++j)
    for (i = 0; i < SCREEN_WIDTH; ++i)
      base[i + j * SCREEN_WIDTH] = clear_char;

  cur_x = 0;
  cur_y = 0;

  fullpos = cur_x + cur_y * SCREEN_WIDTH;
  
  boot_outportb (VIDEO_CRTC_INDEX, VIDEO_CRTC_INDEX_CURSOR_LOCATION_LOW);
  boot_outportb (VIDEO_CRTC_DATA, fullpos & 0xff);

  boot_outportb (VIDEO_CRTC_INDEX, VIDEO_CRTC_INDEX_CURSOR_LOCATION_HIGH);
  boot_outportb (VIDEO_CRTC_DATA, (fullpos >> 8) & 0xff);
}

void
boot_puts (const char *str)
{
  schar thischar = {' ', BOOTTIME_DEFAULT_ATTRIBUTE};
  schar *base = (schar *) VIDEO_BASE;
  uint16_t fullpos;
  
  while (*str)
  {
    if (*str == '\n')
    {
      cur_x = 0;
      ++cur_y;

      if (cur_y == EGA_DEFAULT_SCREEN_HEIGHT)
        cur_y = 0;
    }
    else
    {
      thischar.glyph = *str;
      base[cur_x++ + cur_y * SCREEN_WIDTH].glyph = *str;

      if (cur_x == SCREEN_WIDTH)
      {
        cur_x = 0;
        ++cur_y;

        if (cur_y == EGA_DEFAULT_SCREEN_HEIGHT)
          cur_y = 0;
      }
    }

    ++str;
  }

  fullpos = cur_x + cur_y * SCREEN_WIDTH;
  
  boot_outportb (VIDEO_CRTC_INDEX, VIDEO_CRTC_INDEX_CURSOR_LOCATION_LOW);
  boot_outportb (VIDEO_CRTC_DATA, fullpos & 0xff);

  boot_outportb (VIDEO_CRTC_INDEX, VIDEO_CRTC_INDEX_CURSOR_LOCATION_HIGH);
  boot_outportb (VIDEO_CRTC_DATA, (fullpos >> 8) & 0xff);
}

static void
dword_to_decimal (uint32_t num, char *out)
{
  int i, p;
  char temp;
  
  p = 0;

  if (!num)
  {
    *out++ = '0';
    *out++ = '\0';
  }
  else
  {
    while (num)
    {
      out[p++] = (num % 10) + '0';
      num /= 10;
    }
    
    for (i = 0; i < p / 2; ++i)
    {
      temp = out[i];
      out[i] = out[p - i - 1];
      out[p - i - 1] = temp;
    }

    out[p] = '\0';
  }
}

static void
dword_to_hex (uint32_t num, char *out)
{
  int i, p, n;
  char temp;
  char hexchars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  p = 0;
  n = 8;
    
  while (n--)
  {
    out[p++] = hexchars[num & 0xf];
    num >>= 4;
  }
    
  for (i = 0; i < p / 2; ++i)
  {
    temp = out[i];
    out[i] = out[p - i - 1];
    out[p - i - 1] = temp;
  }

  out[p] = '\0';
  
}

void
boot_print_hex (uint32_t num)
{
  char buf[20];

  dword_to_hex (num, buf);

  boot_puts (buf);
}

void
boot_print_dec (uint32_t num)
{
  char buf[20];
  
  dword_to_decimal (num, buf);

  boot_puts (buf);
}

void
boot_halt (void)
{
  for (;;)
    __asm__ __volatile__ ("hlt");
}

struct page_table
{
  uint32_t entries[PAGE_SIZE / sizeof (uint32_t)];
};


  
static void
boot_print_mmap (uint32_t phys, uint32_t virt, uint32_t pages)
{
  char msg0[] = " 0x";
  char msg1[] = " --> 0x";
  char msg2[] = " (";
  char msg3[] = " pages)\n";

  boot_puts (msg0);
  boot_print_hex (phys);
  boot_puts (msg1);
  boot_print_hex (virt);
  boot_puts (msg2);
  boot_print_dec (pages);
  boot_puts (msg3);
}

static void
boot_setup_vregion (uint32_t page_phys_start, uint32_t page_virt_start, uint32_t page_count)
{
  uint32_t j;
  uint32_t page_phys, page_virt;
  struct page_table *current;

  boot_print_mmap (page_phys_start << 12, page_virt_start << 12, page_count);
  
  for (j = 0; j < page_count; ++j)
  {
    page_phys = j + page_phys_start;
    page_virt = j + page_virt_start;
    
    if (!page_dir->entries[page_virt >> 10])
    {
      current = page_table_list + page_table_count++;
      page_dir->entries[page_virt >> 10] = (uint32_t) current | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
    }
    else
      current = (struct page_table *) (page_dir->entries[page_virt >> 10] & PAGE_MASK);
    
    current->entries[page_virt & 1023] = (page_phys << 12) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
  }
}

void
boot_prepare_paging_early (void)
{
  uint32_t free_mem = __ALIGN ((uint32_t) &kernel_end, PAGE_SIZE);
  
  struct multiboot_info *mbi;
  struct memory_map *mmap_info;
    
  uint32_t page_count;
  uint32_t page_phys_start;
  uint32_t page_virt_start;
  
  uint32_t i;
  uint32_t mmap_count;
  char errmsg[] = "No memory maps in MBI!";

  struct module *mod;
  
  mbi = multiboot_location ();

  if (!(mbi->flags & (1 << 6)))
  {
    boot_puts (errmsg);
    boot_halt ();
  }

  if (mbi->mods_count)
  {
    mod = (struct module *) mbi->mods_addr;
    
    initrd_phys  = (void *) mod->mod_start;
    initrd_size  = mod->mod_end - mod->mod_start;

    if (mod->mod_end > free_mem)
      free_mem = __ALIGN (mod->mod_end, PAGE_SIZE);
  }
  
  page_dir = (struct page_table *) free_mem;
  page_table_list = page_dir + 1;
  page_table_count = 0;
  
  for (i = 0; i < PAGE_SIZE / sizeof (uint32_t); ++i)
    page_dir->entries[i] = 0;

  mmap_count = mbi->mmap_length / sizeof (memory_map_t);
  
  /* Map microkernel to upperhalf */
  boot_setup_vregion ((uint32_t) &kernel_start >> 12, (uint32_t) &text_start >> 12, __UNITS (free_mem - (uint32_t) &kernel_start, PAGE_SIZE));

  /* Map video memory */
  boot_setup_vregion ((uint32_t) VIDEO_BASE >> 12, (uint32_t) VIDEO_BASE >> 12, 1);

  /* Remap initrd to upperhalf aswell (if any) */
  if (initrd_size > 0)
    initrd_start = (void *) ((uint32_t) initrd_phys + PAGE_START ((uint32_t) &text_start) - PAGE_START ((uint32_t) &kernel_start));
  
  for (i = 0; i < mmap_count; ++i)
  {    
    mmap_info = (struct memory_map *) (mbi->mmap_addr + i * sizeof (memory_map_t));

    page_count = __UNITS (mmap_info->length_low, PAGE_SIZE);
    
    page_phys_start = mmap_info->base_addr_low >> 12;
    page_virt_start = page_phys_start;

    boot_setup_vregion (page_phys_start, page_virt_start, page_count);
  }

  __free_start = (uint32_t) &page_table_list[page_table_count];
}

void
boot_fix_multiboot (void)
{
  int i;
  char *p;
  
  struct multiboot_info *mbi;

  mbi = multiboot_location ();

  if (mbi->flags & (1 << 2))
  {
    p = (char *) mbi->cmdline;
    i = 0;
    
    while (*p && i < sizeof (cmdline_copy) - 1)
      cmdline_copy[i++] = *p++;

    cmdline_copy[i] = '\0';

    mbi->cmdline = (unsigned long) cmdline_copy;
  }
}

void
boot_entry (void)
{
  uint32_t cr0;
  
  boot_screen_clear (BOOTTIME_DEFAULT_ATTRIBUTE);
  
  boot_puts (string0);
  
  boot_puts (string1);
  boot_print_hex ((uint32_t) &kernel_start);
  boot_puts (string2);
  boot_print_hex ((uint32_t) &kernel_end);
  boot_puts (string3);
  boot_print_dec ((uint32_t) &kernel_end - (uint32_t) &kernel_start);
  boot_puts (string4);
  boot_puts (string5);
  boot_print_hex ((uint32_t) &text_start);
  boot_puts (string6);

  boot_prepare_paging_early ();

  boot_fix_multiboot ();
  
  boot_puts (string7);
  
  SET_REGISTER ("%cr3", page_dir);
  
  GET_REGISTER ("%cr0", cr0);
  
  cr0 |= CR0_PAGING_ENABLED;
  
  SET_REGISTER ("%cr0", cr0);

  boot_screen_clear (0x07);
  
  main ();
}
