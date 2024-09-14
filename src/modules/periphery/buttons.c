#include "buttons.h"

#include <pico/binary_info.h>

static void button_init(uint gpio /*, gpio_irq_callback_t callback*/)
{
  gpio_init(gpio);
  gpio_set_dir(gpio, GPIO_IN);
  gpio_set_pulls(gpio, true, false);
  gpio_set_input_hysteresis_enabled(gpio, true);
  // gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);
}

void buttons_init(/*gpio_irq_callback_t callback*/)
{
  // clang-format off
  bi_decl_if_func_used(bi_program_feature("User Buttons"))
  bi_decl_if_func_used(bi_2pins_with_names(buttons_gpio_1, "Button 1", buttons_gpio_2, "Button 2"))
    // clang-format on

    button_init(buttons_gpio_1 /*, callback*/);
  button_init(buttons_gpio_2 /*, callback*/);
}

bool buttons_is_pressed(uint button_gpio) { return !gpio_get(button_gpio); }
