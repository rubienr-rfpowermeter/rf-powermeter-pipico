#pragma once
#include "AverageT.hpp"

using Average20uint16  = AverageT<20, uint16_t, uint32_t>;
using Average50uint16  = AverageT<50, uint16_t, uint32_t>;
using Average100uint16 = AverageT<100, uint16_t, uint32_t>;
using Average255uint16 = AverageT<255, uint16_t, uint32_t>;

using AverageUint16 = Average255uint16;
using ResultUint16  = ResultT<uint16_t>;
