#include "joystick.h"

#include "stdio.h"
#include <hardware/gpio.h>
#include <pico/binary_info.h>

static uint8_t state;

static uint8_t mask_from_gpio(uint8_t gpio)
{
  switch(gpio)
  {
  case JOYSTICK_GPIO_UP:
    return JOYSTICK_MASK_UP;
  case JOYSTICK_GPIO_DOWN:
    return JOYSTICK_MASK_DOWN;
  case JOYSTICK_GPIO_LEFT:
    return JOYSTICK_MASK_LEFT;
  case JOYSTICK_GPIO_RIGHT:
    return JOYSTICK_MASK_RIGHT;
  case JOYSTICK_GPIO_Z:
    return JOYSTICK_MASK_Z;

  default:
    return 0;
  }
}

bool joystick_on_edge_cb(uint gpio, uint32_t event_mask)
{
  const uint8_t gpio_mask = { mask_from_gpio(gpio) };

  if ( 0 == gpio_mask) return false;

  else if(event_mask & GPIO_IRQ_EDGE_RISE)
  {
    // button is released
    state &= ~gpio_mask;
    return true;
  }
  else if(event_mask & GPIO_IRQ_EDGE_FALL)
  {
    // button is pressed
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

void joystick_init()
{
  // clang-format off
  bi_decl_if_func_used(bi_program_feature("Joystick"))
  bi_decl_if_func_used(bi_3pins_with_names(
    JOYSTICK_GPIO_UP,   "joystick up",
    JOYSTICK_GPIO_DOWN, "joystick down",
    JOYSTICK_GPIO_LEFT, "joystick left"))
  bi_decl_if_func_used(bi_2pins_with_names(
    JOYSTICK_GPIO_RIGHT,"joystick right",
    JOYSTICK_GPIO_Z,    "joystick z"))
    // clang-format on

    state = 0;
  init_gpio(JOYSTICK_GPIO_UP);
  init_gpio(JOYSTICK_GPIO_DOWN);
  init_gpio(JOYSTICK_GPIO_LEFT);
  init_gpio(JOYSTICK_GPIO_RIGHT);
  init_gpio(JOYSTICK_GPIO_Z);

  gpio_set_irq_enabled(JOYSTICK_GPIO_UP, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_enabled(JOYSTICK_GPIO_DOWN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_enabled(JOYSTICK_GPIO_LEFT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_enabled(JOYSTICK_GPIO_RIGHT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_enabled(JOYSTICK_GPIO_Z, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

bool joystick_any_active(uint8_t mask) { return 0 != (state & mask); }
bool joystick_all_active(uint8_t mask) { return mask == (state & mask); }

uint8_t joystick_get_mask() { return state; }

bool joystick_any_active_in_mask(uint8_t joystick_flags, uint8_t state_mask) { return 0 != (joystick_flags & state_mask); }
bool joystick_all_active_in_mask(uint8_t joystick_flags, uint8_t state_mask) { return joystick_flags == (joystick_flags & state_mask); };
