#pragma once

#include <pico/types.h>

void joystick_init();

bool joystick_on_edge_cb(uint gpio, uint32_t event_mask);

uint8_t joystick_get_mask();
