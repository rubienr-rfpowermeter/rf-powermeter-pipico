#pragma once

#include <pico/types.h>

constexpr uint8_t JOYSTICK_GPIO_UP    = { 2 };
constexpr uint8_t JOYSTICK_GPIO_DOWN  = { 18 };
constexpr uint8_t JOYSTICK_GPIO_LEFT  = { 16 };
constexpr uint8_t JOYSTICK_GPIO_RIGHT = { 20 };
constexpr uint8_t JOYSTICK_GPIO_Z     = { 3 };

constexpr uint8_t JOYSTICK_MASK_UP    = { 1 << 0 };
constexpr uint8_t JOYSTICK_MASK_DOWN  = { 1 << 1 };
constexpr uint8_t JOYSTICK_MASK_LEFT  = { 1 << 2 };
constexpr uint8_t JOYSTICK_MASK_RIGHT = { 1 << 3 };
constexpr uint8_t JOYSTICK_MASK_Z     = { 1 << 4 };
constexpr uint8_t JOYSTICK_MASK_ALL = { JOYSTICK_MASK_UP | JOYSTICK_MASK_DOWN | JOYSTICK_MASK_LEFT | JOYSTICK_MASK_RIGHT | JOYSTICK_MASK_Z };


#ifdef __cplusplus
extern "C"
{
#endif

void joystick_init();

bool joystick_on_edge_cb(uint gpio, uint32_t event_mask);

bool joystick_any_active(uint8_t mask);
bool joystick_all_active(uint8_t mask);

uint8_t joystick_get_mask();

bool joystick_any_active_in_mask(uint8_t joystick_flags, uint8_t state_mask);
bool joystick_all_active_in_mask(uint8_t joystick_flags, uint8_t state_mask);

#ifdef __cplusplus
} // extern "C"
#endif
