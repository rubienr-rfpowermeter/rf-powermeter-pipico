#pragma once
#include "AverageT.hpp"
#include <limits.h>
namespace avg
{

using Average32x16 = AverageT<32, uint16_t, uint32_t, 0, UINT16_MAX>;

} // namespace avg
