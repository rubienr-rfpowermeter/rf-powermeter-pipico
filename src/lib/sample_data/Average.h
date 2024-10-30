#pragma once
#include "AverageT.hpp"

constexpr uint8_t AVERAGE_COUNT{ 255 };

using Average = AverageT<AVERAGE_COUNT, uint16_t, uint32_t>;
using Sample  = ResultT<uint16_t>;
