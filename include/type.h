#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

#define FLAG_ON  0x1
#define FLAG_OFF 0x0

#define FLAG_HIT 0x1
#define FLAG_MISS 0x0

#define KB      8 * 1024
#define BYTE    8

u16 _log2(u16 x);

#endif

