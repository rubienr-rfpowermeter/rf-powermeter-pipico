#pragma once

constexpr uint8_t JOYSTICK_MASK_UP{ 1 << 0 };
constexpr uint8_t JOYSTICK_MASK_DOWN{ 1 << 1 };
constexpr uint8_t JOYSTICK_MASK_LEFT{ 1 << 2 };
constexpr uint8_t JOYSTICK_MASK_RIGHT{ 1 << 3 };
constexpr uint8_t JOYSTICK_MASK_Z{ 1 << 4 };
constexpr uint8_t JOYSTICK_MASK_ALL{ JOYSTICK_MASK_UP | JOYSTICK_MASK_DOWN | JOYSTICK_MASK_LEFT | JOYSTICK_MASK_RIGHT | JOYSTICK_MASK_Z };
