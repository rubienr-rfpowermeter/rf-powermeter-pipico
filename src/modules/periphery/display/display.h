#pragma once

#include "display_types.h"

#ifdef __cplusplus

extern "C"
{
#endif

void display_init();
DmaPeriphery &display_get_dma_periphery();
void lcd_set_window(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y);
#ifdef __cplusplus
} // extern "C"
#endif
