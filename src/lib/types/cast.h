#pragma once

#include <stdint.h>

union __packed As32BitView
{
  uint32_t int32;
  int32_t  uint32;
};

union __packed As16BitView
{
  int16_t  int16;
  uint16_t uint16;
};

struct __packed As2x16BitView
{
  As16BitView l;
  As16BitView h;
};

union __packed As8BitView
{
  int8_t  int8;
  uint8_t uint8;
};

struct __packed As4x8BitView
{
  As8BitView b0;
  As8BitView b1;
  As8BitView b2;
  As8BitView b3;
};

//! 32-bit type for lazy casts
union IntCast
{
  As32BitView   as_32bit;
  As2x16BitView as_16bit;
  As4x8BitView  as_8bit;
  float         as_float;
};

static_assert(sizeof(IntCast) == 4, "error: size mismatch");
