#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

//! Initialize low level display driver
void lv_port_disp_init(void);

//! Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
void disp_enable_update(void);

//! Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
void disp_disable_update(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif
