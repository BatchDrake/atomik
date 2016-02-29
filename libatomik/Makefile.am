AUTOMAKE_OPTIONS = subdir-objects

LIBATOMIK_ALLSOURCES =        \
        include/system/call.h \
        include/system/cap.h  \
        src/cap.c 

LIBATOMIK_I386 = arch/i386/cap.c

noinst_LIBRARIES = libatomik.a

libatomik_a_CFLAGS =-std=c99 -nostdinc -nostdlib -fno-builtin -I../musl/src/include -I../musl/include -Iinclude -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith -D_XOPEN_SOURCE=700 -ggdb -I../musl/arch/@AM_ARCH@ @AM_CFLAGS@

libatomik_a_SOURCES = $(LIBATOMIK_ALLSOURCES) $(LIBATOMIK_I386)
