AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libtcb.a
libtcb_a_CFLAGS =-std=c99 -nostdinc -nostdlib -fno-builtin -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith -D_XOPEN_SOURCE=700 -Iinclude -I../include -I../../musl/include -I../arch/@AM_ARCH@/include -ggdb @AM_CFLAGS@

libtcb_a_SOURCES = elf.c tcb.c
