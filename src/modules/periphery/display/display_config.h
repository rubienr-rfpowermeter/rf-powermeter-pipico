#pragma once

#include "display_hw_config.h"
#include "display_types.h"

constexpr uint16_t DISPLAY_NUM_PIXELS = { DISPLAY_HORIZONTAL_PX * DISPLAY_VERTICAL_PX };

constexpr enum DisplayScanDirection DISPLAY_DIRECTION = { DisplayScanDirection_90_DEG };

/// display dimension as seen by SW (after rotation)
constexpr uint16_t DISPLAY_WIDTH_PX = { DisplayScanDirection_0_DEG == DISPLAY_DIRECTION ? DISPLAY_HORIZONTAL_PX : DISPLAY_VERTICAL_PX };
/// display dimension as seen by SW (after rotation)
constexpr uint16_t DISPLAY_HEIGHT_PX = { DisplayScanDirection_0_DEG == DISPLAY_DIRECTION ? DISPLAY_VERTICAL_PX : DISPLAY_HORIZONTAL_PX };

constexpr uint8_t DISPLAY_BACKLIGHT_DEFAULT_PERCENT = { 70 };
constexpr uint8_t DISPLAY_BACKLIGHT_MIN_PERCENT     = { 1 };
