#include "user_led.h"

#include "user_led_hw_config.h"
#include <hardware/gpio.h>
#include <pico/binary_info.h>

void user_led_init()
{
  // clang-format off
  bi_decl_if_func_used(bi_program_feature("User LED"))
  bi_decl_if_func_used(bi_1pin_with_name(USER_LED_GPIO, "user LED"))
    // clang-format on

    gpio_init(USER_LED_GPIO);
  gpio_set_pulls(USER_LED_GPIO, false, false);
  gpio_set_dir(USER_LED_GPIO, GPIO_OUT);
  gpio_clr_mask(1 << USER_LED_GPIO);
}

void user_led_set(bool on) { gpio_put(USER_LED_GPIO, on); }

void user_led_toggle() { gpio_xor_mask(1 << USER_LED_GPIO); }
