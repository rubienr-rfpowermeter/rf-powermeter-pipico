#pragma once

#include <pico/types.h>

void buttons_init();

bool buttons_on_state_changed_cb(uint gpio, uint32_t event_mask);

uint8_t buttons_get_mask();
