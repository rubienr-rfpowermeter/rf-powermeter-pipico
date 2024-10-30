#pragma once

#include <gitmodules/lvgl/src/misc/lv_types.h>

struct TrackedInputs;
typedef struct TrackedInputs TrackedInputs;

void lv_input_init(TrackedInputs &keys);
void lv_input_deinit(TrackedInputs &keys);

lv_group_t *lv_input_get_buttons_group();
lv_group_t *lv_input_get_keypad_group();
