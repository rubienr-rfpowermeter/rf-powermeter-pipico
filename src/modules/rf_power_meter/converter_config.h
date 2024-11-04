#pragma once

#include "converter_types.h"

namespace rfpm
{

const float AD7887_V_REF{ 2.5 };       /// AD7887 reverence voltage in [V]
const float AD8138_MAX_OUT_V{ 2.1 };   /// AD8138 max output voltage in [V]
const float AD8138_MIN_OUT_V{ 0.5 };   /// AD7887 min output voltage in [V]

constexpr BandSpecs AD8318_BAND_SPECS[FrequencyBand::BandsTotal]{
  [FrequencyBand::GHz_0_9] = {  .f_GHz_em1 = 9, .slope_mV_em1 = -245, .intercept_dBm_em1 = 220, .min_input_level_dBm = -58, .max_input_level_dBm = -1,  .r_Tadj_ohm = 500 },
  [FrequencyBand::GHz_1_9] = { .f_GHz_em1 = 19, .slope_mV_em1 = -244, .intercept_dBm_em1 = 224, .min_input_level_dBm = -59, .max_input_level_dBm = -2,  .r_Tadj_ohm = 500 },
  [FrequencyBand::GHz_2_2] = { .f_GHz_em1 = 22, .slope_mV_em1 = -244, .intercept_dBm_em1 = 196, .min_input_level_dBm = -60, .max_input_level_dBm = -2,  .r_Tadj_ohm = 500 },
  [FrequencyBand::GHz_5_8] = { .f_GHz_em1 = 58, .slope_mV_em1 = -243,  .intercept_dBm_em1 = 25, .min_input_level_dBm = -58, .max_input_level_dBm = -1, .r_Tadj_ohm = 1000 },
  [FrequencyBand::GHz_8_0] = { .f_GHz_em1 = 80, .slope_mV_em1 = -230, .intercept_dBm_em1 = 270, .min_input_level_dBm = -55, .max_input_level_dBm = -3,  .r_Tadj_ohm = 500 },
};

}   // namespace rfpm
