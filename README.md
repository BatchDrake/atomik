Atomik's refactorization
===
This GitHub repository belongs to the last refactorization of Atomik. The goals are clear: apply all the gathered knowledge from the previous project (Neftis) to a well-designed, maintainable and fast microkernel. As Neftis had a rather convoluted directory structure, the refactorization will affect it as well. The following document describes how is it designed, where to place new files and how.

Overview
---
The tree consists of the following directories and subdirectories:

    musl/
        arch/
        include/
        src/
    src/
        arch/
            i386/
                include/
        include/
        
The subdirectory `musl/` contains a subset of the [musl C library](http://www.musl-libc.org/), whose details are provided below and it shouldn't be modified unless there is a shortcoming in it preventing you from coding. The subdirectory `src/` will contain the microkernel code itself, having its entry point (its 'main' function) in `src/main.c`. Each component (like TCB, endpoint, vspace, etc) will be contained in a subdirectory inside this folder (e.g. `src/objmgr`) along with its include directories (`src/objmgr/include`).

Architecture-dependant code is stored in subdirectories under `src/arch`. Things like accessing the serial port, flushing the TLB, context switching implementation, etc should be contained here. Currently, only `src/arch/i386` exists but a few others will also be added once Atomik is ported to additional architectures.

Adding new components to Atomik
--
As previously described, all components should be created as subdirectories under `src/` and be compiled as static libraries against the microkernel. For a good example, let's consider a generic component called `component` (and therefore, contained in the `src/component` subdirectory) with 4 files: `file1.c`, `file2.c`, `include/file1.h` and `include/file2.h`. The component directory must have a `Makefile.am` file with the following contents:

```
AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libcomponent.a
libcomponent_a_CFLAGS =-std=c99 -nostdinc -nostdlib -fno-builtin -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith -D_XOPEN_SOURCE=700 -Iinclude -I../include -I../../musl/include -I../arch/@AM_ARCH@/include -ggdb @AM_CFLAGS@

libcomponent_a_SOURCES = file1.c include/file1.h file2.c include/file2.h
```

Then, both the component library and the component subdirectory should be referenced in `src/Makefile.am` as follows:

- Add `component/` to the `SUBDIRS` variable. Each subdirectory is separated by spaces.
- Add `component/libcomponent.a` to `atomik_LDADD`, right before `-lgcc`.

After that, you should update `configure.ac` by telling it to generate a new Makefile. Just add `src/component/Makefile` to the `AC_OUTPUT` command located at the end of the file.

> **Remark:**
> for any configuration, it's important that all Makefiles define *at least* the following compiler flags:
> 
>  `-nostdinc -nostdlib -fno-builtin -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith`
     
> This will ensure that no implicit function calls will be allowed (ensuring code safety and consistency, as any function being called from a different file will have to be declared in a header file first), neither implicit return types defaulting to int (functions must have a well-defined return type) nor `void *` pointer arithmetics (this is, adding or substracting to a `void *` pointer). Also, we remove any interference with the standard include files, libraries or builtin functions with `-nostdinc -nostdlib -fno-builtin`.

Compilation
--
The procedure to compile atomik is as follows (please note that not all steps are required depending on the files being modified):

    % autoreconf -fvi # Required only if configure.ac or any of the Makefile.am files have been modified, or right after checking out from the GitHub repository.
    % ./configure # Required only the first time Atomik is being compiled, or if  CFLAGS/CCASFLAGS/ARCH environment variables have been modified.
    % make

Any additional compiler flags can be passed through the `CFLAGS` environment variable to the `./configure` script. Assembler flags can also be passed through the `CCASFLAGS`. This script accepts the `ARCH` environment variable, used to define the target architecture. If not provided, the target architecture is `i386`.

Standard C library subset
--
Atomik includes a subset of the standard C library. This subset only comprises standard output to stdout and strings (printf, sprintf and so on), stdlib functions and macros, string, basic math, integers, wide chars, errno and ctype. Please note that although many functions are exposed in the header files, it doesn't mean they are available (like `fopen()`). Further work on this subset is necessary in order to remove all unnecesary prototypes.

The full subset of the C library header files that can be used in Atomik is:
- `#include <alloca.h>`
- `#include <alltypes.h>`
- `#include <complex.h>`
- `#include <ctype.h>`
- `#include <endian.h>`
- `#include <errno.h>`
- `#include <features.h>`
- `#include <float.h>`
- `#include <inttypes.h>`
- `#include <limits.h>`
- `#include <math.h>`
- `#include <memory.h>`
- `#include <stdalign.h>`
- `#include <stdarg.h>`
- `#include <stdbool.h>`
- `#include <stddef.h>`
- `#include <stdint.h>`
- `#include <stdio_ext.h>`
- `#include <stdio.h>`
- `#include <stdlib.h>`
- `#include <stdnoreturn.h>`
- `#include <string.h>`
- `#include <strings.h>`
- `#include <stropts.h>`
- `#include <uchar.h>`
- `#include <unistd.h>`
- `#include <values.h>`
- `#include <wchar.h>`
- `#include <wctype.h>`

