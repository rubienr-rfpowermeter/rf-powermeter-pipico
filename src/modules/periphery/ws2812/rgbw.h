#pragma once

#include "hardware/pio.h"
#include <inttypes.h>

extern PIO rgbw_pio;
extern int rgbw_sm;
extern int rgbw_offset;

#ifdef __cplusplus
extern "C"
{
#endif

void rgbw_put_pixel(uint32_t pixel_grb);
uint32_t rgbw_to_u32(uint8_t r, uint8_t g, uint8_t b);
void rgbw_init(void);

#ifdef __cplusplus
} // extern "C"
#endif
