#pragma once

#include <pico/types.h>

constexpr uint buzzer_gpio = {13};

#ifdef __cplusplus
extern "C"
{
#endif

void buzzer_init(void);
void buzzer_beep(void);

#ifdef __cplusplus
} // extern "C"
#endif
