OUTPUT_FORMAT("elf32-i386")
ENTRY (start)

kernel_base = 0xd0000000;

SECTIONS{
    /DISCARD/ : { *(.note.gnu.build-id) }
    /DISCARD/ : { *(.note.gnu.gold-version) }

    . = 0x00100000;
    
    .entry :
    {
      *(.entry)
    }
    .bootcode :
    {
        *(.bootcode)
    }

    .bootdata :
    {
        *(.bootdata)
    }

    . = ALIGN (0x1000);

    kernel_start = .;

    .text kernel_base : AT (kernel_start)
    {
      text_start = .;
      *(.text)
      . = ALIGN (0x1000);
      text_end = .;
    }

    .data : AT (ADDR (.data) - kernel_base + kernel_start)
    {
      *(.data)
    }

    .rodata : AT (ADDR (.rodata) - kernel_base + kernel_start)
    {
      *(.rodata)
    }

    .bss : AT (ADDR (.bss) - kernel_base + kernel_start)
    {
      bss_start = .;
      *(COMMON)
      *(.bss)
      bss_end = .;
    }

    debugsyms : AT (ADDR (debugsyms) - kernel_base + kernel_start)
    {
	__start_debugsyms = .;
	*(debugsyms)
	__stop_debugsyms = .;
    }
    . = ALIGN (0x1000);
    
    kernel_end = . - kernel_base + kernel_start;
}

