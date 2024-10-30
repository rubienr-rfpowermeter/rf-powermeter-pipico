#pragma once

struct TrackedInputs;
typedef struct TrackedInputs TrackedInputs;

struct _lv_group_t;
typedef struct _lv_group_t lv_group_t;

void lv_input_init(TrackedInputs &keys);
lv_group_t *lv_input_get_buttons_group();
lv_group_t *lv_input_get_keypad_group();
