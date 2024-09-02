#include "leds.h"

#include <hardware/gpio.h>
#include <pico/binary_info.h>


void user_leds_init(void)
{
    bi_decl_if_func_used(bi_program_feature("USER LEDs (2x)"))          //
    bi_decl_if_func_used(bi_1pin_with_name(leds_gpio_1, "LED1 (blue)")) //
    bi_decl_if_func_used(bi_1pin_with_name(leds_gpio_2, "LED2 (blue)")) //

    gpio_init(leds_gpio_1);
    gpio_init(leds_gpio_2);
    gpio_set_dir(leds_gpio_1, GPIO_OUT);
    gpio_set_dir(leds_gpio_2, GPIO_OUT);

    gpio_put(leds_gpio_1, 0);
    gpio_put(leds_gpio_2, 0);
}
