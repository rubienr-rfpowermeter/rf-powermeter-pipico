#pragma once

#include <gitmodules/lvgl/src/misc/lv_types.h>

struct TrackedInputs;

void lv_input_init(TrackedInputs &keys);
void lv_input_deinit();

lv_group_t *lv_input_get_buttons_group();
lv_group_t *lv_input_get_keypad_group();
