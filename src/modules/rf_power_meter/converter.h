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
   printf("xxx raw %hu res %f\n", raw_value, (v_ref * static_cast<UnderlyingConversionType>(raw_value)) / (4096 - 1));
  return (v_ref * static_cast<UnderlyingConversionType>(raw_value)) / (4096 - 1);
}

constexpr UnderlyingConversionType dbvFromAdc(float voltFromAdc, FrequencyBand band)
{
  const UnderlyingConversionType slope_mV{
    static_cast<UnderlyingConversionType>(AD8318_BAND_SPECS[std::to_underlying<FrequencyBand>(band)].slope_mV_em1) * 0.1f
  };

  if (voltFromAdc < AD8138_MIN_OUT_V) { printf("xxx v %f\n", voltFromAdc); }
  else if (voltFromAdc > AD8138_MAX_OUT_V) { printf("xxx ^ %f\n", voltFromAdc); }

  return (voltFromAdc < AD8138_MIN_OUT_V) ? std::numeric_limits<UnderlyingConversionType>::max() :
         (voltFromAdc > AD8138_MAX_OUT_V) ? std::numeric_limits<UnderlyingConversionType>::min() :
                                            (AD8138_MAX_OUT_V - voltFromAdc) * slope_mV;
}

/// Corrects the raw dBmW value according to the correction coefficients (k-values) and attenuation offset.
/// @param rawValue the value to convert
/// @refitem converted and corrected value
constexpr UnderlyingConversionType dbvCorrectedFromDbv(UnderlyingConversionType dbvFromAdc, float attenuationDb, const CorrectionValues &correction)
{
  const UnderlyingConversionType v{ static_cast<UnderlyingConversionType>(dbvFromAdc) };
  const UnderlyingConversionType v2 = { v * v };
  const UnderlyingConversionType v3 = { v2 * v };
  return attenuationDb + correction.k0 + correction.k1 * v + correction.k2 * v2 + correction.k3 * v3;
}

/// Converts the dBmW value to Watts using SI units.
/// @param correctedDbmW the dBmW value to convert to W
constexpr SiUnit toLinearV(const UnderlyingConversionType &correctedDbv)
{
  const UnderlyingConversionType milliWatt = powf(10.0f, correctedDbv / 10.0f);
  constexpr si::Linearity        l{ si::Linearity::Linear };
  constexpr si::Unit             u{ si::Unit::Watt };

  if (milliWatt < 0.000000001f) return { .value = milliWatt * 1e12f, .lin = l, .scale = si::Scale::Femto, .unit = u };
  else if (milliWatt < 0.000001f) return { .value = milliWatt * 1e9f, .lin = l, .scale = si::Scale::Pico, .unit = u };
  else if (milliWatt < 0.001f) return { .value = milliWatt * 1e6f, .lin = l, .scale = si::Scale::Nano, .unit = u };
  else if (milliWatt < 1.0f) return { .value = milliWatt * 1e3f, .lin = l, .scale = si::Scale::Micro, .unit = u };
  else if (milliWatt < 1000.0f) return { .value = milliWatt * 10e0f, .lin = l, .scale = si::Scale::Milli, .unit = u };
  else return { .value = milliWatt * 10e-3f, .lin = l, .scale = si::Scale::TimesOne, .unit = u };
}

}   // namespace rfpm
