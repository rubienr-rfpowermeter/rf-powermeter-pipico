#include "lv_input.h"

#include "modules/periphery/display/display_hw_config.h"
#include "modules/periphery/input/buttons.h"
#include "modules/periphery/input/input.h"
#include "modules/periphery/input/joystick.h"
#include <gitmodules/lvgl/src/indev/lv_indev.h>
#include <stdio.h>

static lv_indev_t    *indev_keypad{ nullptr };
static lv_group_t    *indev_keypad_group{ nullptr };
static lv_indev_t    *indev_button{ nullptr };
static lv_group_t    *indev_button_group{ nullptr };
static TrackedInputs *input_keys{ nullptr };

static void on_read_keys(__unused lv_indev_t *indev, lv_indev_data_t *data)
{
  input_keys->update_joystick(joystick_get_mask());
  static uint8_t last_key = 0;

  data->state = input_keys->joystick_active ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
  if (input_keys->next_active_axis) last_key = input_keys->next_active_axis->id;

  data->key              = last_key;
  data->continue_reading = false;
}

static void on_read_buttons(__unused lv_indev_t *indev, lv_indev_data_t *data)
{
  input_keys->update_buttons(buttons_get_mask());
  static uint8_t last_button{ 0 };

  data->state = input_keys->button_active ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
  if (nullptr != input_keys->next_active_button) last_button = input_keys->next_active_button->id;

  data->btn_id           = last_button;
  data->continue_reading = false;
}

void lv_input_init(TrackedInputs &keys)
{
  input_keys = &keys;

  {
    indev_keypad = lv_indev_create();
    lv_indev_set_type(indev_keypad, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev_keypad, on_read_keys);

    indev_keypad_group = lv_group_create();
    lv_indev_set_group(indev_keypad, indev_keypad_group);
  }

  {
    indev_button = lv_indev_create();
    lv_indev_set_type(indev_button, LV_INDEV_TYPE_BUTTON);
    lv_indev_set_read_cb(indev_button, on_read_buttons);

    static constexpr lv_point_t points_array[]{
      { DISPLAY_HORIZONTAL_PX - 10, 0 * (DISPLAY_VERTICAL_PX / 4) + (DISPLAY_VERTICAL_PX / 8) },
      { DISPLAY_HORIZONTAL_PX - 10, 1 * (DISPLAY_VERTICAL_PX / 4) + (DISPLAY_VERTICAL_PX / 8) },
      { DISPLAY_HORIZONTAL_PX - 10, 2 * (DISPLAY_VERTICAL_PX / 4) + (DISPLAY_VERTICAL_PX / 8) },
      { DISPLAY_HORIZONTAL_PX - 10, 3 * (DISPLAY_VERTICAL_PX / 4) + (DISPLAY_VERTICAL_PX / 8) }
    };

    lv_indev_set_button_points(indev_button, points_array);

    indev_button_group = lv_group_create();
    lv_indev_set_group(indev_button, indev_button_group);
  }
}

void lv_input_deinit()
{
  input_keys = nullptr;

  lv_group_remove_all_objs(indev_keypad_group);
  lv_group_delete(indev_keypad_group);
  lv_indev_delete(indev_keypad);
  indev_keypad_group = nullptr;
  indev_keypad       = nullptr;

  lv_group_remove_all_objs(indev_button_group);
  lv_group_delete(indev_button_group);
  lv_indev_delete(indev_button);
  indev_button_group = nullptr;
  indev_button       = nullptr;
}

lv_group_t *lv_input_get_buttons_group() { return indev_button_group; }

lv_group_t *lv_input_get_keypad_group() { return indev_keypad_group; }
