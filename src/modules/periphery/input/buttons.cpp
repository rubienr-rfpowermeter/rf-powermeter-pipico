#include "buttons.h"

#include "buttons_hw_config.h"
#include "buttons_types.h"
#include <hardware/gpio.h>
#include <pico/binary_info.h>

static volatile uint8_t state;

static uint8_t mask_from_gpio(uint8_t gpio)
{
  switch(gpio)
  {
  case BUTTONS_GPIO_A:
    return BUTTONS_MASK_A;
  case BUTTONS_GPIO_B:
    return BUTTONS_MASK_B;
  case BUTTONS_GPIO_X:
    return BUTTONS_MASK_X;
  case BUTTONS_GPIO_Y:
    return BUTTONS_MASK_Y;

  default:
    return 0;
  }
}

bool buttons_on_state_changed_cb(uint gpio, uint32_t event_mask)
{
  const uint8_t gpio_mask = { mask_from_gpio(gpio) };

  if(0 == gpio_mask) return false;

  else if(event_mask & GPIO_IRQ_EDGE_RISE) // on released
  {
    state &= ~gpio_mask;
    return true;
  }
  else if(event_mask & GPIO_IRQ_EDGE_FALL) // on pressed
  {
    state |= gpio_mask;
    return true;
  }

  return false;
}

static void init_gpio(uint gpio)
{
  gpio_init(gpio);
  gpio_pull_up(gpio);
  gpio_set_input_hysteresis_enabled(gpio, true);
}

void buttons_init()
{
  // clang-format off
  bi_decl_if_func_used(bi_program_feature("User Buttons"))
  bi_decl_if_func_used(bi_4pins_with_names(
    BUTTONS_GPIO_A, "button A",
    BUTTONS_GPIO_B, "button B",
    BUTTONS_GPIO_X, "button X",
    BUTTONS_GPIO_Y, "button Y"))
    // clang-format on

    state = 0;
  init_gpio(BUTTONS_GPIO_A);
  init_gpio(BUTTONS_GPIO_B);
  init_gpio(BUTTONS_GPIO_X);
  init_gpio(BUTTONS_GPIO_Y);

  gpio_set_irq_enabled(BUTTONS_GPIO_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_enabled(BUTTONS_GPIO_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_enabled(BUTTONS_GPIO_X, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_enabled(BUTTONS_GPIO_Y, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

uint8_t buttons_get_mask() { return state; }
