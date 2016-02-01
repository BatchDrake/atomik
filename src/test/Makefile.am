AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libtest.a
libtest_a_CFLAGS =-std=c99 -nostdinc -nostdlib -fno-builtin -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith -D_XOPEN_SOURCE=700 -Iinclude -I../include -I../../musl/include -I../arch/@AM_ARCH@/include -ggdb @AM_CFLAGS@

libtest_a_SOURCES = test.c include/test.h
