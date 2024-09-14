#include "buzzer.h"

#include <hardware/gpio.h>
#include <pico/binary_info.h>

void buzzer_init()
{
  // clang-format off
  bi_decl_if_func_used(bi_program_feature("Buzzer"))
  bi_decl_if_func_used(bi_1pin_with_name(buzzer_gpio, "Buzzer"))
    // clang-format on

    gpio_init(buzzer_gpio);
  gpio_disable_pulls(buzzer_gpio);
  gpio_set_drive_strength(buzzer_gpio, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_slew_rate(buzzer_gpio, GPIO_SLEW_RATE_SLOW);
  gpio_set_dir(buzzer_gpio, GPIO_OUT);
}

void buzzer_toggle_beep() { gpio_xor_mask(1 << buzzer_gpio); }

void buzzer_beep(bool do_beep) { gpio_put(buzzer_gpio, do_beep); }
