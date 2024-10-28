#pragma once

#include "hardware/dma.h"
#include <pico/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum LcdColor : uint16_t
{
  LcdColor_WHITE      = 0xFFFF,
  LcdColor_BLACK      = 0x0000,
  LcdColor_BLUE       = 0x001F,
  LcdColor_BRED       = 0XF81F,
  LcdColor_GRED       = 0XFFE0,
  LcdColor_GBLUE      = 0X07FF,
  LcdColor_RED        = 0xF800,
  LcdColor_MAGENTA    = 0xF81F,
  LcdColor_GREEN      = 0x07E0,
  LcdColor_CYAN       = 0x7FFF,
  LcdColor_YELLOW     = 0xFFE0,
  LcdColor_BROWN      = 0XBC40,
  LcdColor_BRRED      = 0XFC07,
  LcdColor_GRAY       = 0X8430,
  LcdColor_DARKBLUE   = 0X01CF,
  LcdColor_LIGHTBLUE  = 0X7D7C,
  LcdColor_GRAYBLUE   = 0X5458,
  LcdColor_LIGHTGREEN = 0X841F,
  LcdColor_LGRAY      = 0XC618,
  LcdColor_LGRAYBLUE  = 0XA651,
  LcdColor_LBBLUE     = 0X2B12,
};

enum LcdScanDirection : uint8_t
{
  LcdScanDirection_0_DEG,
  LcdScanDirection_90_DEG,
};

typedef struct
{
  uint32_t backlight_slice_nr;
  uint32_t tx_dma_channel;
  dma_channel_config tx_dma_config;
} DmaPeriphery;

static_assert(sizeof(LcdColor) == 2);
static_assert(sizeof(LcdScanDirection) == 1);

#ifdef __cplusplus
} // extern "C"
#endif
