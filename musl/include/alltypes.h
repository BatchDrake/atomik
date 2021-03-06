#ifndef _BITS_ALLTYPES_H
#include <bits/alltypes.h>
#endif

#ifndef _ALLTYPES_H
#define _ALLTYPES_H

typedef unsigned _Addr size_t;
typedef unsigned _Addr uintptr_t;
typedef _Addr ptrdiff_t;
typedef _Addr ssize_t;
typedef _Addr intptr_t;
typedef _Addr regoff_t;
typedef _Reg register_t;

typedef signed char     int8_t;
typedef short           int16_t;
typedef int             int32_t;
typedef _Int64          int64_t;
typedef _Int64          intmax_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned _Int64 uint64_t;
typedef unsigned _Int64 u_int64_t;
typedef unsigned _Int64 uintmax_t;

typedef unsigned mode_t;
typedef unsigned _Reg nlink_t;
typedef _Int64 off_t;
typedef unsigned _Int64 ino_t;
typedef unsigned _Int64 dev_t;
typedef long blksize_t;
typedef _Int64 blkcnt_t;
typedef unsigned _Int64 fsblkcnt_t;
typedef unsigned _Int64 fsfilcnt_t;

typedef unsigned wint_t;
typedef unsigned long wctype_t;

typedef void * timer_t;
typedef int clockid_t;
typedef long clock_t;
struct timeval { time_t tv_sec; suseconds_t tv_usec; };
struct timespec { time_t tv_sec; long tv_nsec; };

typedef int pid_t;
typedef unsigned id_t;
typedef unsigned uid_t;
typedef unsigned gid_t;
typedef int key_t;
typedef unsigned useconds_t;

#ifdef __cplusplus
typedef unsigned long pthread_t;
#else
typedef struct __pthread * pthread_t;
#endif
typedef int pthread_once_t;
typedef unsigned pthread_key_t;
typedef int pthread_spinlock_t;
typedef struct { unsigned __attr; } pthread_mutexattr_t;
typedef struct { unsigned __attr; } pthread_condattr_t;
typedef struct { unsigned __attr; } pthread_barrierattr_t;
typedef struct { unsigned __attr[2]; } pthread_rwlockattr_t;

typedef struct _IO_FILE FILE;

typedef struct __mbstate_t { unsigned __opaque1, __opaque2; } mbstate_t;

typedef struct __locale_struct * locale_t;

typedef struct __sigset_t { unsigned long __bits[128/sizeof(long)]; } sigset_t;

struct iovec { void *iov_base; size_t iov_len; };

typedef unsigned socklen_t;
typedef unsigned short sa_family_t;

#endif
