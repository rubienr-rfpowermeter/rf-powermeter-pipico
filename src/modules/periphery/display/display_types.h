#pragma once

#include "hardware/dma.h"
#include <pico/types.h>

enum DisplayColor : uint16_t
{
  DisplayColor_WHITE      = 0xFFFF,
  DisplayColor_BLACK      = 0x0000,
  DisplayColor_BLUE       = 0x001F,
  DisplayColor_BRED       = 0XF81F,
  DisplayColor_GRED       = 0XFFE0,
  DisplayColor_GBLUE      = 0X07FF,
  DisplayColor_RED        = 0xF800,
  DisplayColor_MAGENTA    = 0xF81F,
  DisplayColor_GREEN      = 0x07E0,
  DisplayColor_CYAN       = 0x7FFF,
  DisplayColor_YELLOW     = 0xFFE0,
  DisplayColor_BROWN      = 0XBC40,
  DisplayColor_BRRED      = 0XFC07,
  DisplayColor_GRAY       = 0X8430,
  DisplayColor_DARKBLUE   = 0X01CF,
  DisplayColor_LIGHTBLUE  = 0X7D7C,
  DisplayColor_GRAYBLUE   = 0X5458,
  DisplayColor_LIGHTGREEN = 0X841F,
  DisplayColor_LGRAY      = 0XC618,
  DisplayColor_LGRAYBLUE  = 0XA651,
  DisplayColor_LBBLUE     = 0X2B12,
};

enum DisplayScanDirection : uint8_t
{
  DisplayScanDirection_0_DEG,
  DisplayScanDirection_90_DEG,
};

struct DmaPeriphery
{
  uint32_t           backlight_slice_nr;
  uint32_t           tx_dma_channel;
  dma_channel_config tx_dma_config;
};

static_assert(sizeof(DisplayColor) == 2);
static_assert(sizeof(DisplayScanDirection) == 1);
