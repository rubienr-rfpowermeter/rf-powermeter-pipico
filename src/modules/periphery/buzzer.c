#include "buzzer.h"

#include <hardware/gpio.h>
#include <pico/binary_info.h>

void buzzer_init(void)
{
    bi_decl_if_func_used(bi_program_feature("Buzzer"))             //
    bi_decl_if_func_used(bi_1pin_with_name(buzzer_gpio, "Buzzer")) //
    gpio_init(buzzer_gpio);
    gpio_set_dir(buzzer_gpio, GPIO_OUT);
}

void buzzer_beep(void) { gpio_xor_mask(1 << buzzer_gpio); }
