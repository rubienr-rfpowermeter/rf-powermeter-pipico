#pragma once

#include <pico/types.h>

const constexpr uint buzzer_gpio = {13};

void buzzer_init(void);
void buzzer_beep(void);
