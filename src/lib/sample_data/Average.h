#pragma once
#include "AverageT.hpp"
#include "lib/si/si.h"

using Average255Uint16 = AverageT<255, uint16_t, uint32_t>;
using AveragedUint16 = ResultT<uint16_t>;
