/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

#define bittab __fsmu8

#include <stdint.h>

#ifdef __PIC__
__attribute__((__visibility__("hidden")))
#endif
extern const uint32_t bittab[];

/* Upper 6 state bits are a negative integer offset to bound-check next byte */
/*    equivalent to: ( (b-0x80) | (b+offset) ) & ~0x3f      */
#define OOB(c,b) (((((b)>>3)-0x10)|(((b)>>3)+((int32_t)(c)>>26))) & ~7)

/* Interval [a,b). Either a must be 80 or b must be c0, lower 3 bits clear. */
#define R(a,b) ((uint32_t)((a==0x80 ? 0x40u-b : 0u-a) << 23))
#define FAILSTATE R(0x80,0x80)

#define SA 0xc2u
#define SB 0xf4u

/* Arbitrary encoding for representing code units instead of characters. */
#define CODEUNIT(c) (0xdfff & (signed char)(c))
#define IS_CODEUNIT(c) ((unsigned)(c)-0xdf80 < 0x80)

#include <stdlib.h>
#include "libc.h"
