#pragma once


#include <pico/types.h>

constexpr uint8_t BUTTONS_GPIO_A = { 15 };
constexpr uint8_t BUTTONS_GPIO_B = { 17 };
constexpr uint8_t BUTTONS_GPIO_X = { 19 };
constexpr uint8_t BUTTONS_GPIO_Y = { 21 };

constexpr uint8_t BUTTONS_MASK_A   = { 1 << 0 };
constexpr uint8_t BUTTONS_MASK_B   = { 1 << 1 };
constexpr uint8_t BUTTONS_MASK_X   = { 1 << 2 };
constexpr uint8_t BUTTONS_MASK_Y   = { 1 << 3 };
constexpr uint8_t BUTTONS_MASK_ALL = { BUTTONS_MASK_A | BUTTONS_MASK_B | BUTTONS_MASK_X | BUTTONS_MASK_Y };


#ifdef __cplusplus
extern "C"
{
#endif

void buttons_init();

bool buttons_on_state_changed_cb(uint gpio, uint32_t event_mask);

bool buttons_all_pressed(uint8_t mask);
bool buttons_any_pressed(uint8_t mask);

uint8_t buttons_get_mask();

bool buttons_all_pressed_in_mask(uint8_t button_flags, uint8_t state_mask);
bool buttons_any_pressed_in_mask(uint8_t button_flags, uint8_t state_mask);

#ifdef __cplusplus
} // extern "C"
#endif
