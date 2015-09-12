AUTOMAKE_OPTIONS = subdir-objects

EXTRA_DIST = kernel.lds

noinst_LIBRARIES = libi386.a

libi386_a_CFLAGS =-std=c99 -nostdinc -nostdlib -fno-builtin -Iinclude -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith -D_XOPEN_SOURCE=700 -I../../include -I../../../musl/include -ggdb @AM_CFLAGS@

libi386_a_CCASFLAGS = -nostdinc -nostdlib -fno-builtin -Iinclude -I../../include -I../../../musl/include -ggdb @AM_CCASFLAGS@

libi386_a_SOURCES = \
	arch.c \
	boot.c \
	boot-i386.S \
	serial.c \
	include/i386-io.h \
	include/i386-layout.h \
	include/i386-page.h \
	include/i386-regs.h \
	include/i386-serial.h \
	include/i386-vga.h \
	include/machinedefs.h \
	include/multiboot.h 
