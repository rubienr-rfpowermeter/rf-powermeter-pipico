#pragma once

#include <pico/types.h>

const constexpr uint leds_gpio_1 = {16};
const constexpr uint leds_gpio_2 = {17};

void user_leds_init(void);
