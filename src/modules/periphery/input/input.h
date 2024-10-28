#pragma once

#include <pico/types.h>

typedef struct TrackedInputState
{
  uint8_t is_pressed : 1;
  uint8_t is_released : 1;
  uint8_t is_event : 1;
  uint8_t rfu : 3; // reserved
  uint8_t counter;

  void update(bool signal);
  TrackedInputState();

} TrackedInputState;

typedef struct TrackedInputs
{
  TrackedInputState a;
  TrackedInputState b;
  TrackedInputState x;
  TrackedInputState y;
  TrackedInputState up;
  TrackedInputState down;
  TrackedInputState left;
  TrackedInputState right;
  TrackedInputState z;

  bool joystick_active;
  bool joystick_event;
  bool button_active;
  bool button_event;

  void update(uint8_t buttons_signal_mask, uint8_t joystick_signal_mask);
  void print() const;

} TrackedInputs;

void input_init();
bool input_any_active(uint8_t mask, uint8_t state_flags);
bool input_all_active(uint8_t mask, uint8_t state_flags);
