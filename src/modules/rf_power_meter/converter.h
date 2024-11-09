#pragma once
#include "converter_config.h"
#include "converter_types.h"
#include "lib/si/si.h"

/// Converter for power detector module with AD8318 + ADC AD7887
/// AD8318 - 1 MHz to 8 GHz, 70 dB Logarithmic Detector/Controller

namespace rfpm
{

using namespace si;

constexpr UnderlyingConversionType voltFrom12bitAdc(uint16_t raw_value, UnderlyingConversionType v_ref)
{
  return (v_ref * static_cast<UnderlyingConversionType>(raw_value)) / static_cast<float>(4096 - 1);
}

constexpr UnderlyingConversionType dbvFromVAdc(float voltFromAdc, FrequencyBand band)
{
  const UnderlyingConversionType slope_V{
    static_cast<UnderlyingConversionType>(AD8318_BAND_SPECS[std::to_underlying<FrequencyBand>(band)].slope_mV_em1) * 0.1f * 0.001f
  };

  const UnderlyingConversionType min_input_level_db{ static_cast<UnderlyingConversionType>(
    AD8318_BAND_SPECS[std::to_underlying<FrequencyBand>(band)].min_input_level_dBm) };

  return min_input_level_db + ((voltFromAdc > AD8318_MAX_OUT_V) ? 0 :
                               (voltFromAdc < AD8318_MIN_OUT_V) ? (AD8318_MAX_OUT_V - AD8318_MIN_OUT_V) / fabsf(slope_V) :
                                                                  ((AD8318_MAX_OUT_V - voltFromAdc) / fabsf(slope_V)));
}

/// Corrects the raw dBmW value according to the correction coefficients (k-values) and attenuation offset.
/// @param rawValue the value to convert
/// @refitem converted and corrected value
constexpr UnderlyingConversionType dbvCorrectedFromDbv(UnderlyingConversionType dbvFromAdc, const CorrectionValues &correction)
{
  const UnderlyingConversionType v{ dbvFromAdc };
  const UnderlyingConversionType v2 = { v * v };
  const UnderlyingConversionType v3 = { v2 * v };
  return correction.k0 + correction.k1 * v + correction.k2 * v2 + correction.k3 * v3;
}

/// Converts the dBmW value to Watts using SI units.
/// @param correctedDbmW the dBmW value to convert to W
constexpr SiUnit toLinearV(const UnderlyingConversionType &correctedDbv)
{
  const UnderlyingConversionType mV = powf(10.0f, correctedDbv / 10.0f);
  constexpr si::Linearity        l{ si::Linearity::Linear };
  constexpr si::Unit             u{ si::Unit::Volt };

  if (mV < 0.000000001f) return { .value = mV * 1e12f, .lin = l, .scale = si::Scale::Femto, .unit = u };
  else if (mV < 0.000001f) return { .value = mV * 1e9f, .lin = l, .scale = si::Scale::Pico, .unit = u };
  else if (mV < 0.001f) return { .value = mV * 1e6f, .lin = l, .scale = si::Scale::Nano, .unit = u };
  else if (mV < 1.0f) return { .value = mV * 1e3f, .lin = l, .scale = si::Scale::Micro, .unit = u };
  else if (mV < 1000.0f) return { .value = mV * 10e0f, .lin = l, .scale = si::Scale::Milli, .unit = u };
  else return { .value = mV * 10e-3f, .lin = l, .scale = si::Scale::TimesOne, .unit = u };
}

}   // namespace rfpm
