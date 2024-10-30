#include "lv_input.h"

#include "lvgl.h"
#include "modules/periphery/input/buttons.h"
#include "modules/periphery/input/input.h"
#include "modules/periphery/input/joystick.h"
#include <stdio.h>

static lv_indev_t *indev_keypad       = { nullptr };
static lv_group_t *indev_keypad_group = { nullptr };
static lv_indev_t *indev_button       = { nullptr };
static lv_group_t *indev_button_group = { nullptr };
static TrackedInputs *input_keys      = { nullptr };

static void on_read_keys(lv_indev_drv_t __unused *indev_drv, lv_indev_data_t *data)
{
  input_keys->update_joystick(joystick_get_mask());
  static uint8_t last_key = 0;

  data->state = input_keys->joystick_active ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
  if(input_keys->next_active_axis) last_key = input_keys->next_active_axis->id;

  data->key              = last_key;
  data->continue_reading = false;
}

static void on_read_buttons(lv_indev_drv_t __unused *indev_drv, lv_indev_data_t *data)
{
  input_keys->update_buttons(buttons_get_mask());
  static uint8_t last_button = { 0 };

  data->state = input_keys->button_active ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
  if(nullptr != input_keys->next_active_button) last_button = input_keys->next_active_button->id;

  data->btn_id           = last_button;
  data->continue_reading = false;
}

void lv_input_init(TrackedInputs &keys)
{
  input_keys = &keys;

  static lv_indev_drv_t input_dev_drv_key;
  lv_indev_drv_init(&input_dev_drv_key);
  input_dev_drv_key.type    = LV_INDEV_TYPE_KEYPAD;
  input_dev_drv_key.read_cb = on_read_keys;
  indev_keypad              = lv_indev_drv_register(&input_dev_drv_key);

  indev_keypad_group = lv_group_create();
  lv_indev_set_group(indev_keypad, indev_keypad_group);

  static lv_indev_drv_t input_dev_drv_button;
  lv_indev_drv_init(&input_dev_drv_button);
  input_dev_drv_button.type    = LV_INDEV_TYPE_BUTTON;
  input_dev_drv_button.read_cb = on_read_buttons;
  indev_button                 = lv_indev_drv_register(&input_dev_drv_button);

  static const lv_point_t points_array[] = {
    { 220 + 10, 0 * 60 + 30 },
    { 220 + 10, 1 * 60 + 30 },
    { 220 + 10, 2 * 60 + 30 },
    { 220 + 10, 3 * 60 + 30 }
  };

  lv_indev_set_button_points(indev_button, points_array);

  indev_button_group = lv_group_create();
  lv_indev_set_group(indev_button, indev_button_group);
}

lv_group_t *lv_input_get_buttons_group() { return indev_button_group; }

lv_group_t *lv_input_get_keypad_group() { return indev_keypad_group; }
