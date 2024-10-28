#pragma once

#include "display_hw_config.h"
#include "display_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LCD_SPI_PORT (spi1)

constexpr uint16_t LCD_NUM_PIXELS = { LCD_HORIZONTAL_PX * LCD_VERTICAL_PX };

constexpr enum LcdScanDirection LCD_DIRECTION = { LcdScanDirection_90_DEG };

/// display dimension as seen by SW (after rotation)
constexpr uint16_t LCD_WIDTH_PX = { LcdScanDirection_0_DEG == LCD_DIRECTION ? LCD_HORIZONTAL_PX : LCD_VERTICAL_PX };
/// display dimension as seen by SW (after rotation)
constexpr uint16_t LCD_HEIGHT_PX = { LcdScanDirection_0_DEG == LCD_DIRECTION ? LCD_VERTICAL_PX : LCD_HORIZONTAL_PX };

constexpr uint8_t LCD_BACKLIGHT_DEFAULT_PERCENT = { 70 };
constexpr uint8_t LCD_BACKLIGHT_MIN_PERCENT     = { 1 };

#ifdef __cplusplus
} // extern "C"
#endif
