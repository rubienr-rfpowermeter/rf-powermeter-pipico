#pragma once

#include "display_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LCD_SPI_PORT (spi1)

constexpr uint8_t LCD_GPIO_DC        = { 8 };   /// data/command select; high: data, low: command
constexpr uint8_t LCD_GPIO_CS        = { 9 };   /// chip select; low: active
constexpr uint8_t LCD_GPIO_CLK       = { 10 };  /// SPI display clock input
constexpr uint8_t LCD_GPIO_MOSI      = { 11 };  /// SPI display input
constexpr uint8_t LCD_GPIO_RST       = { 12 };  /// display reset; low: active
constexpr uint8_t LCD_GPIO_BL        = { 13 };  /// display backlight PWM input
constexpr uint16_t LCD_HORIZONTAL_PX = { 240 }; /// display dimension
constexpr uint16_t LCD_VERTICAL_PX   = { 240 }; /// display dimension

#ifdef __cplusplus
} // extern "C"
#endif
