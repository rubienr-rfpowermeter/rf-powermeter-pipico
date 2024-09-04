#pragma once

#include <hardware/gpio.h>
#include <pico/types.h>

constexpr uint joystick_axis_x = {26};
constexpr uint joystick_axis_y = {27};

#ifdef __cplusplus
extern "C"
{
#endif

void joystick_init(gpio_irq_callback_t callback);

#ifdef __cplusplus
} // extern "C"
#endif
