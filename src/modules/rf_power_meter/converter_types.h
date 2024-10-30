#pragma once
#include "KValues3rdOrder.hpp"
#include "lib/si/si.h"

namespace rfpm
{

using UnderlyingConversionType = float;
using CorrectionValues         = KValues3rdOrder<UnderlyingConversionType>;
using SiUnit                     = si::Value<UnderlyingConversionType>;

enum FrequencyBand : uint8_t
{
  GHz_0_9 = 0,
  GHz_1_9,
  GHz_2_2,
  GHz_5_8,
  GHz_8_0,
  BandsTotal
};

struct BandSpecs
{
  uint16_t f_GHz_em1;             /// [GHz] * 10^-1 frequency domain
  int16_t  slope_mV_em1;          /// [mV] * 10^-1
  int16_t  intercept_dBm_em1;     /// [dBm] * 10^-1
  int16_t  min_input_level_dBm;   /// [dBm] min input level at +/-1dB error
  int16_t  max_input_level_dBm;   /// [dBm] max input level at +/-1dB error
  uint16_t r_Tadj_ohm;            /// [Ω]
};

}   // namespace rfpm
