#pragma once

#include <hardware/gpio.h>
#include <pico/types.h>

const constexpr uint buttons_gpio_1 = {15};
const constexpr uint buttons_gpio_2 = {14};

#ifdef __cplusplus
extern "C"
{
#endif

void buttons_init(gpio_irq_callback_t callback);

#ifdef __cplusplus
} // extern "C"
#endif
