#pragma once

#include "modules/periphery/input/input.h"
#ifdef __cplusplus
extern "C"
{
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
  #include "lvgl.h"
#else
  #include "lvgl/lvgl.h"
#endif

void ui_init();
void ui_update_from_peripherals(const TrackedInputs &keys);

#ifdef __cplusplus
} // extern "C"
#endif
