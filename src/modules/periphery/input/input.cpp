#include "input.h"

#include "buttons_types.h"
#include "joystick_types.h"
#include <cinttypes>
#include <pico/stdlib.h>
#include <stdio.h>

void input_init() { irq_set_enabled(IO_IRQ_BANK0, true); }

void input_deinit() { irq_set_enabled(IO_IRQ_BANK0, false); }

bool input_any_active(uint8_t mask, uint8_t state_flags) { return 0 != (mask & state_flags); }

bool input_all_active(uint8_t mask, uint8_t state_flags) { return mask == (mask & state_flags); }

TrackedInputState::TrackedInputState(uint8_t id) : is_pressed(0), is_released(0), is_event(0), rfu(0), counter(0), id(id) { }

bool TrackedInputState::update(bool signal)
{
  if (signal) counter = counter << 1 | 0x01;
  else counter >>= 1;

  const bool is_now_pressed{ counter > 0b00010000 };
  const bool is_now_released{ counter < 0b00001000 };

  is_event = (is_now_pressed && !is_now_released && !is_pressed) || (is_now_released && !is_now_pressed && !is_released);

  is_pressed  = is_now_pressed;
  is_released = is_now_released;

  return is_event;
}

void TrackedInputs::update_buttons(uint8_t buttons_signal_mask)
{
  next_active_button = nullptr;
  next_active_button = a.update(input_any_active(BUTTONS_MASK_A, buttons_signal_mask)) ? &a : next_active_button;
  next_active_button = b.update(input_any_active(BUTTONS_MASK_B, buttons_signal_mask)) ? &b : next_active_button;
  next_active_button = x.update(input_any_active(BUTTONS_MASK_X, buttons_signal_mask)) ? &x : next_active_button;
  next_active_button = y.update(input_any_active(BUTTONS_MASK_Y, buttons_signal_mask)) ? &y : next_active_button;

  button_event  = a.is_event + b.is_event + x.is_event + y.is_event;
  button_active = a.is_pressed + b.is_pressed + x.is_pressed + y.is_pressed;
}

void TrackedInputs::update_joystick(uint8_t joystick_signal_mask)
{
  next_active_axis = nullptr;
  next_active_axis = up.update(input_any_active(JOYSTICK_MASK_UP, joystick_signal_mask)) ? &up : next_active_axis;
  next_active_axis = down.update(input_any_active(JOYSTICK_MASK_DOWN, joystick_signal_mask)) ? &down : next_active_axis;
  next_active_axis = left.update(input_any_active(JOYSTICK_MASK_LEFT, joystick_signal_mask)) ? &left : next_active_axis;
  next_active_axis = right.update(input_any_active(JOYSTICK_MASK_RIGHT, joystick_signal_mask)) ? &right : next_active_axis;
  next_active_axis = z.update(input_any_active(JOYSTICK_MASK_Z, joystick_signal_mask)) ? &z : next_active_axis;

  joystick_event  = up.is_event + down.is_event + left.is_event + right.is_event + z.is_event;
  joystick_active = up.is_pressed + down.is_pressed + left.is_pressed + right.is_pressed + z.is_pressed;
}

void TrackedInputs::print() const
{
  // clang-format off
  printf(
    "a=%" PRIu8 "%" PRIu8 "%" PRIu8 " "
    "b=%" PRIu8 "%" PRIu8 "%" PRIu8 " "
    "x=%" PRIu8 "%" PRIu8 "%" PRIu8 " "
    "y=%" PRIu8 "%" PRIu8 "%" PRIu8 " "
    "u=%" PRIu8 "%" PRIu8 "%" PRIu8 " "
    "d=%" PRIu8 "%" PRIu8 "%" PRIu8 " "
    "l=%" PRIu8 "%" PRIu8 "%" PRIu8 " "
    "r=%" PRIu8 "%" PRIu8 "%" PRIu8 " "
    "z=%" PRIu8 "%" PRIu8 "%" PRIu8 " "
    "bs=%" PRIu8 "%" PRIu8 " "
    "js=%" PRIu8 "%" PRIu8 "\n",
    a.is_pressed,     a.is_released,     a.is_event,
    b.is_pressed,     b.is_released,     b.is_event,
    x.is_pressed,     x.is_released,     x.is_event,
    y.is_pressed,     y.is_released,     y.is_event,
    up.is_pressed,    up.is_released,    up.is_event,
    down.is_pressed,  down.is_released,  down.is_event,
    left.is_pressed,  left.is_released,  left.is_event,
    right.is_pressed, right.is_released, right.is_event,
    z.is_pressed,     z.is_released,     z.is_event,
    button_active, button_event,
    joystick_active, joystick_event
  );
  // clang-format on
}
