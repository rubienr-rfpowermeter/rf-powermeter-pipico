#pragma once

#include <cinttypes>

struct DmaPeriphery;

void display_init();
DmaPeriphery &display_get_dma_periphery();
void display_set_window(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y);
