#pragma once

#include <cinttypes>

template <typename value_type> struct ResultT;
using ResultUint16 = ResultT<uint16_t>;

void ui_init(ResultUint16 &samples);
void ui_update();
