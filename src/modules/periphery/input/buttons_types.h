#pragma once

constexpr uint8_t BUTTONS_MASK_A   = { 1 << 0 };
constexpr uint8_t BUTTONS_MASK_B   = { 1 << 1 };
constexpr uint8_t BUTTONS_MASK_X   = { 1 << 2 };
constexpr uint8_t BUTTONS_MASK_Y   = { 1 << 3 };
constexpr uint8_t BUTTONS_MASK_ALL = { BUTTONS_MASK_A | BUTTONS_MASK_B | BUTTONS_MASK_X | BUTTONS_MASK_Y };
