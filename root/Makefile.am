bin_PROGRAMS = root

root_LDADD= ../musl/libmusluser.a ../libatomik/libatomik.a -lgcc # GCC, I hate you soooo much. No joke.
root_LDFLAGS=@AM_LDFLAGS@
root_CFLAGS = -I../musl/include -I../libatomik/include -ggdb -nostdinc -nostdlib -fno-builtin -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith @AM_CFLAGS@
root_CCASFLAGS = @AM_CFLAGS@

root_SOURCES = main.c

