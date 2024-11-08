#pragma once

#include <pico/stdlib.h>

#define AD7887_SPI_PORT (spi0)             /// AD7887 SPI port
constexpr uint8_t AD7887_GPIO_DOUT{ 4 };   /// AD7887 output
constexpr uint8_t AD7887_GPIO_CS{ 5 };     /// AD7887 chip select; low: active
constexpr uint8_t AD7887_GPIO_SCLK{ 6 };   /// AD7887 clock input
constexpr uint8_t AD7887_GPIO_DIN{ 7 };    /// AD7887 input
