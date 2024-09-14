#pragma once

#include <pico/types.h>

constexpr uint buzzer_gpio = {13};

#ifdef __cplusplus
extern "C"
{
#endif

void buzzer_init();
void buzzer_toggle_beep();
void buzzer_beep(bool do_beep);

#ifdef __cplusplus
} // extern "C"
#endif
