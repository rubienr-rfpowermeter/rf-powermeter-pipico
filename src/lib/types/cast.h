#pragma once

#include <stdint.h>

typedef union __packed
{
  uint32_t int32;
  int32_t uint32;
} As32BitView;

typedef union __packed
{
  int16_t int16;
  uint16_t uint16;
} As16BitView;

typedef struct __packed
{
  As16BitView l;
  As16BitView h;

} As2x16BitView;

typedef union __packed
{
  int8_t int8;
  uint8_t uint8;
} As8BitView;

typedef struct __packed
{
  As8BitView b0;
  As8BitView b1;
  As8BitView b2;
  As8BitView b3;
} As4x8BitView;

//! 32-bit type for lazy casts
typedef union
{
  As32BitView as_32bit;
  As2x16BitView as_16bit;
  As4x8BitView as_8bit;
  float as_float;
} IntCast;

static_assert(sizeof(IntCast) == 4, "error: size mismatch");
