#pragma once

#include <hardware/spi.h>
#include <pico/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
  #include "lvgl.h"
#else
  #include "lvgl/lvgl.h"
#endif

void lv_display_init();
void lv_display_deinit();

#ifdef __cplusplus
} // extern "C"
#endif
