#pragma once
#include "AverageT.hpp"

constexpr uint8_t AVERAGE_COUNT {255};
using UnderlyingAverageType = double;

using Average = AverageT<AVERAGE_COUNT, UnderlyingAverageType, UnderlyingAverageType>;
using Sample  = ResultT<UnderlyingAverageType>;
