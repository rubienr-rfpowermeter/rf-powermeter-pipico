#pragma once

#include <cinttypes>

namespace ad7887
{

constexpr float voltFrom12bitAdc(uint16_t raw_value, float v_ref) { return v_ref * static_cast<float>(raw_value) / 4095.0f; }

}   // namespace ad7887
