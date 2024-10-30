#pragma once
#include "../si_units/SiUnit.h"
#include "KValues3rdOrder.hpp"

/// Converter for power detector AD8318
/// AD8318 - 1 MHz to 8 GHz, 70 dB Logarithmic Detector/Controller

namespace ad8318
{

using namespace si;

using UnderlyingConversionType = double;
using CorrectionValues         = KValues3rdOrder<UnderlyingConversionType>;

struct LinearW
{
  UnderlyingConversionType watt{ NAN };
  SiUnit                   unit{ si::SiUnit::Undefined };
};

struct Converter3rdOrder
{
  explicit Converter3rdOrder(const CorrectionValues &kValues, UnderlyingConversionType attenuationDb = 0);

  /// Retrieve the actual k-values.
  /// @param[out] kValues
  void getCorrectionCoefficients(CorrectionValues &kValues) const;

  /// Set the k-values for upcoming conversions.
  /// @param kValues
  void setCorrectionCoefficients(const CorrectionValues &kValues);

  void setAttenuationDb(UnderlyingConversionType newAttenuationDb);

  [[nodiscard]] UnderlyingConversionType getAttenuationDb() const;

  /// Corrects the raw dBmW value according to the correction coefficients (k-values) and attenuation offset.
  /// @param rawValue the value to convert
  /// @refitem converted and corrected value
  [[nodiscard]] UnderlyingConversionType toCorrectedDbmW(uint16_t rawValue) const;

  /// Converts the dBmW value to Watts using SI units.
  /// @param[in] correctedDbmW the dBmW value to convert to W
  static constexpr LinearW toLinearW(const UnderlyingConversionType &correctedDbmW);

protected:

  CorrectionValues         correction{ 0, 0, 0 };   /// 3rd-order correction values.
  UnderlyingConversionType attenuationDb{ 0 };      /// offset correction
};

constexpr LinearW Converter3rdOrder::toLinearW(const UnderlyingConversionType &correctedDbmW)
{
  const UnderlyingConversionType milliWatt = powf(10.0f, correctedDbmW / 10.0f);

  if (milliWatt < 0.000000001f) return LinearW{ .watt = milliWatt * 1e12f, .unit = SiUnit::Femto };
  else if (milliWatt < 0.000001f) return LinearW{ .watt = milliWatt * 1e9f, .unit = SiUnit::Pico };
  else if (milliWatt < 0.001f) return LinearW{ .watt = milliWatt * 1e6f, .unit = SiUnit::Nano };
  else if (milliWatt < 1.0f) return LinearW{ .watt = milliWatt * 1e3f, .unit = SiUnit::Micro };
  else if (milliWatt < 1000.0f) return LinearW{ .watt = milliWatt * 10e0f, .unit = SiUnit::Milli };
  else return LinearW{ .watt = milliWatt * 10e-3f, .unit = SiUnit::TimesOne };
}

}   // namespace ad8318
