#pragma once

#include <pico/types.h>

constexpr uint leds_gpio_1 = {16};
constexpr uint leds_gpio_2 = {17};

#ifdef __cplusplus
extern "C"
{
#endif

void user_leds_init(void);

#ifdef __cplusplus
} // extern "C"
#endif
