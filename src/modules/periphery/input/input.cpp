#include "input.h"

#include "buttons_types.h"
#include "joystick_types.h"
#include <cstdio>
#include <pico/stdlib.h>

static constexpr char decimal_digit(uint8_t value) { return static_cast<char>('0' + value); }

static char *append_input_state(char *out, char name, const TrackedInputState &state)
{
  *out++ = name;
  *out++ = '=';
  *out++ = decimal_digit(state.is_pressed);
  *out++ = decimal_digit(state.is_released);
  *out++ = decimal_digit(state.is_event);
  *out++ = ' ';
  return out;
}

void input_init() { irq_set_enabled(IO_IRQ_BANK0, true); }

void input_deinit() { irq_set_enabled(IO_IRQ_BANK0, false); }

bool input_any_active(uint8_t mask, uint8_t state_flags) { return 0 != (mask & state_flags); }

bool input_all_active(uint8_t mask, uint8_t state_flags) { return mask == (mask & state_flags); }

TrackedInputState::TrackedInputState(uint8_t id) : is_pressed(0), is_released(0), is_event(0), rfu(0), counter(0), id(id) { }

bool TrackedInputState::update(bool signal)
{
  if (signal) counter = counter << 1 | 0x01;
  else counter >>= 1;

  const bool is_now_pressed { counter > 0b00000001 };
  const bool is_now_released { counter < 0b00000001 };

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
  // Nine six-character input states, a six-character button summary, a
  // five-character joystick summary, and the null terminator.
  static char line[9 * 6 + 6 + 5 + 1];
  char       *out { line };

  out = append_input_state(out, 'a', a);
  out = append_input_state(out, 'b', b);
  out = append_input_state(out, 'x', x);
  out = append_input_state(out, 'y', y);
  out = append_input_state(out, 'u', up);
  out = append_input_state(out, 'd', down);
  out = append_input_state(out, 'l', left);
  out = append_input_state(out, 'r', right);
  out = append_input_state(out, 'z', z);

  *out++ = 'b';
  *out++ = 's';
  *out++ = '=';
  *out++ = decimal_digit(button_active);
  *out++ = decimal_digit(button_event);
  *out++ = ' ';

  *out++ = 'j';
  *out++ = 's';
  *out++ = '=';
  *out++ = decimal_digit(joystick_active);
  *out++ = decimal_digit(joystick_event);
  *out   = '\0';

  puts(line);
}
