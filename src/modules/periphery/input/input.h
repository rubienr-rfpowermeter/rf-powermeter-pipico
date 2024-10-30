#pragma once

#include <cinttypes>

typedef struct TrackedInputState
{
  uint8_t is_pressed : 1;
  uint8_t is_released : 1;
  uint8_t is_event : 1;
  uint8_t rfu : 3;
  uint8_t counter;
  uint8_t id;

  bool update(bool signal);
  TrackedInputState(uint8_t id);

} TrackedInputState;

typedef struct TrackedInputs
{
  TrackedInputState a{ 0x00 };     /// KEY_NEXT
  TrackedInputState b{ 0x01 };     /// KEY_PREV
  TrackedInputState x{ 0x02 };     /// KEY_HOME
  TrackedInputState y{ 0x03 };     /// KEY_END
  TrackedInputState up{ 0x11 };    /// KEY_UP
  TrackedInputState down{ 0x12 };  /// KEY_DOWN
  TrackedInputState left{ 0x13 };  /// KEY_RIGHT
  TrackedInputState right{ 0x14 }; /// KEY_LEFT
  TrackedInputState z{ 0x0a };     /// KEY_ENTER

  uint8_t joystick_active;
  uint8_t joystick_event;
  uint8_t button_active;
  uint8_t button_event;

  TrackedInputState *next_active_button;
  TrackedInputState *next_active_axis;

  void update_buttons(uint8_t buttons_signal_mask);
  void update_joystick(uint8_t joystick_signal_mask);
  void print() const;

} TrackedInputs;

void input_init();
bool input_any_active(uint8_t mask, uint8_t state_flags);
bool input_all_active(uint8_t mask, uint8_t state_flags);
