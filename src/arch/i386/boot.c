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
#include <i386-seg.h>

#include <multiboot.h>

#define BOOTTIME_DEFAULT_ATTRIBUTE 0x1f

/* Global variables. These can be used by the remapped kernel. */
void  *free_start;
size_t free_size;
void  *remap_start;
size_t remap_size;
void  *kernel_virt_start;
void  *kernel_phys_start;
size_t kernel_size;

struct page_table *page_dir;

char kernel_stack[4 * PAGE_SIZE] __attribute__ ((aligned (PAGE_SIZE)));

/* This is a dirty trick: I will remap __tss to the tss address. Downside:
 * The kernel is 4K bigger. So what?
 */
struct tss tss __attribute__ ((aligned ((PAGE_SIZE))));

/* Symbols provided by linker */
extern int kernel_start; /* Physical address */
extern int kernel_end;   /* Physical address */
extern int text_start;   /* Virtual address */

/* This is the only public boot function */
BOOT_FUNCTION (void boot_entry (void));

/* Functions and symbols below this line are static to protect accidental
 * access from arch.c
 */

/* Functions in case we need to output something REALLY REALLY early */
BOOT_FUNCTION (static void boot_halt (void));
BOOT_FUNCTION (static void boot_screen_clear (uint8_t));
BOOT_FUNCTION (static void boot_puts (const char *));
BOOT_FUNCTION (static void boot_print_hex (uint32_t));
BOOT_FUNCTION (static void boot_print_dec (uint32_t));
BOOT_FUNCTION (static void boot_fix_multiboot (void));

/* Symbols required by extern files */
BOOT_SYMBOL (static uint32_t __free_start);
BOOT_SYMBOL (static uint32_t __free_size);
BOOT_SYMBOL (static uint32_t __remap_start);
BOOT_SYMBOL (static uint32_t __remap_size);
BOOT_SYMBOL (static uint32_t __kernel_size);

BOOT_SYMBOL (static void *initrd_phys) = NULL;
BOOT_SYMBOL (static void *initrd_start) = NULL;
BOOT_SYMBOL (static uint32_t initrd_size) = 0;

BOOT_SYMBOL (char bootstack[4 * PAGE_SIZE]); /* Boot stack, as used by _start */
BOOT_SYMBOL (static struct tss __tss __attribute__ ((aligned ((PAGE_SIZE)))));
BOOT_SYMBOL (static char cmdline_copy[128]);
BOOT_SYMBOL (static struct multiboot_info *multiboot_info);

/* Inner state of boot_entry */
BOOT_SYMBOL (static int cur_x) = 0;
BOOT_SYMBOL (static int cur_y) = 0;
BOOT_SYMBOL (struct page_table *boot_page_dir);
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
BOOT_SYMBOL (static char string8[]) = "Multiboot configuration error: can't find high memory region\n";

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

  boot_print_mmap (
      PAGE_ADDRESS (page_phys_start),
      PAGE_ADDRESS (page_virt_start),
      page_count);
  
  for (j = 0; j < page_count; ++j)
  {
    page_phys = j + page_phys_start;
    page_virt = j + page_virt_start;
    
    if (!boot_page_dir->entries[page_virt >> 10])
    {
      current = page_table_list + page_table_count++;
      boot_page_dir->entries[page_virt >> 10] = (uint32_t) current | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
    }
    else
      current = (struct page_table *) PAGE_START (boot_page_dir->entries[page_virt >> 10]);
    
    current->entries[page_virt & 1023] = PAGE_ADDRESS (page_phys) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
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
  uint32_t highest_addr;
  uint32_t highest_kernel_remap_addr;
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
  
  boot_page_dir = (struct page_table *) free_mem;
  page_table_list = boot_page_dir + 1;
  page_table_count = 0;
  
  for (i = 0; i < PAGE_SIZE / sizeof (uint32_t); ++i)
    boot_page_dir->entries[i] = 0;

  mmap_count = mbi->mmap_length / sizeof (memory_map_t);

  /* Map video memory */
  boot_setup_vregion (PAGE_NUMBER (VIDEO_BASE), PAGE_NUMBER (VIDEO_BASE), 1);

  /* Remap initrd to upperhalf aswell (if any) */
  if (initrd_size > 0)
    initrd_start = (void *) ((uint32_t) initrd_phys + PAGE_START ((uint32_t) &text_start) - PAGE_START ((uint32_t) &kernel_start));

  highest_addr = 0;
  
  for (i = 0; i < mmap_count; ++i)
  {    
    mmap_info = (struct memory_map *) (mbi->mmap_addr + i * sizeof (memory_map_t));

    page_count = __UNITS (mmap_info->length_low, PAGE_SIZE);
    
    page_phys_start = PAGE_NUMBER (mmap_info->base_addr_low);
    page_virt_start = page_phys_start;

    /* Find vregion where free_mem is */
    if (mmap_info->base_addr_low <= free_mem &&
        free_mem < (mmap_info->base_addr_low + mmap_info->length_low))
      highest_addr = mmap_info->base_addr_low + mmap_info->length_low;
    
    boot_setup_vregion (page_phys_start, page_virt_start, page_count);
  }

  if (highest_addr == 0)
  {
    boot_puts (string7);
    boot_halt ();
  }

  highest_kernel_remap_addr = highest_addr;

  if (highest_kernel_remap_addr > (KERNEL_BASE + KERNEL_REMAP_MAX))
    highest_kernel_remap_addr = KERNEL_BASE + KERNEL_REMAP_MAX;
  
  /* Map microkernel to upperhalf */
  __kernel_size = __ALIGN (
                      highest_kernel_remap_addr - (uint32_t) &kernel_start,
                      PAGE_SIZE);

  boot_setup_vregion (
      PAGE_NUMBER ((uint32_t) &kernel_start),
      PAGE_NUMBER ((uint32_t) &text_start),
      __UNITS (__kernel_size, PAGE_SIZE));


  /* Overwrite TSS map with the address of __tss */
  boot_setup_vregion (
      PAGE_NUMBER ((uint32_t) &__tss),
      PAGE_NUMBER ((uint32_t) &tss),
      1);
  
  __free_start  = (uint32_t) &page_table_list[page_table_count];
  __free_size   = highest_addr - __free_start;

  __remap_start = __free_start - (uint32_t) &kernel_start + (uint32_t) &text_start;
  __remap_size  = highest_kernel_remap_addr - __free_start;
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
  
  SET_REGISTER ("%cr3", boot_page_dir);
  
  GET_REGISTER ("%cr0", cr0);
  
  cr0 |= CR0_PAGING_ENABLED;
  
  SET_REGISTER ("%cr0", cr0);

  free_start = (void *) __free_start;
  free_size  = (size_t) __free_size;

  remap_start = (void *) __remap_start;
  remap_size  = (size_t) __remap_size;

  kernel_phys_start = (void *) &kernel_start;
  kernel_virt_start = (void *) &text_start;
  kernel_size       = (size_t) __kernel_size;

  boot_screen_clear (0x07);
  
  /* Make this pointer available to the remapped kernel */
  page_dir = boot_page_dir;

  /* Set the kernel stack in TSS */
  i386_set_kernel_stack ((uint32_t) &kernel_stack + sizeof (kernel_stack) - 4);

  /* Perform stack switch & call main */
  __asm__ __volatile__ (
      ".extern main\n"
      "leal (%0), %%esp\n"
      "addl %%eax, %%esp\n"
      "pushl %%ebx\n"
      "pushl %%ecx\n"
      "call main\n"
      : :
        "m" (kernel_stack[0]),
        "a" (sizeof (kernel_stack) - 4),
        "b" (initrd_size),
        "c" (initrd_start));
}
