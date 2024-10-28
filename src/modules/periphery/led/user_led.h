#pragma once

#include <pico/types.h>

constexpr uint8_t USER_LED_GPIO = { 25 };

#ifdef __cplusplus
extern "C"
{
#endif


void user_led_init();
void user_led_set(bool on);
void user_led_toggle();

#ifdef __cplusplus
} // extern "C"
#endif
