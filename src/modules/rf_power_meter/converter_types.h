#pragma once

#include "KValues3rdOrder.hpp"
#include "lib/sample_data/TransactionData.h"
#include "lib/si/si_types.h"
#include "modules/core0_ui/ui.h"

namespace rfpm
{

using UnderlyingConversionType = float;
using CorrectionValues         = KValues3rdOrder<UnderlyingConversionType>;

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

constexpr si::SiFloat frequencyBandToSi(FrequencyBand band)
{
  switch (band)
  {
  case FrequencyBand::GHz_0_9:
    return { .value = 900.0f, .scale = si::Scale::Mega };
  case FrequencyBand::GHz_1_9:
    return { .value = 1.9f, .scale = si::Scale::Giga };
  case FrequencyBand::GHz_2_2:
    return { .value = 2.2f, .scale = si::Scale::Giga };
  case FrequencyBand::GHz_5_8:
    return { .value = 5.8f, .scale = si::Scale::Giga };
  case FrequencyBand::GHz_8_0:
    return { .value = 8.0f, .scale = si::Scale::Giga };
  default:
    return {};
  }
}

}   // namespace rfpm
