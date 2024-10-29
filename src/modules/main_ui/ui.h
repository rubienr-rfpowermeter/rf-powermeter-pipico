#pragma once

struct TrackedInputs;
typedef TrackedInputs TrackedInputs;

void ui_init();
void ui_update_from_peripherals(const TrackedInputs &keys);
