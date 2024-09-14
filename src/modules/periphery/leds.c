#include "leds.h"

#include <hardware/gpio.h>
#include <pico/binary_info.h>

static void init_gpio(uint gpio)
{
  gpio_init(gpio);
  gpio_set_slew_rate(gpio, GPIO_SLEW_RATE_SLOW);
  gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_dir(gpio, GPIO_OUT);
  gpio_put(gpio, false);
}

void user_leds_init(void)
{
  bi_decl_if_func_used(bi_program_feature("USER LEDs (2x)"))            //
    bi_decl_if_func_used(bi_1pin_with_name(leds_gpio_1, "LED1 (blue)")) //
    bi_decl_if_func_used(bi_1pin_with_name(leds_gpio_2, "LED2 (blue)")) //

    init_gpio(leds_gpio_1);
  init_gpio(leds_gpio_2);
}

void user_leds_set(uint leds_gpio, bool state) { gpio_put(leds_gpio, state); }
