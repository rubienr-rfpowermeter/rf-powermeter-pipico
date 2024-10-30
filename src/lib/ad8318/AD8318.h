#pragma once
#include "../si_units/SiUnit.h"
#include "KValues3rdOrder.hpp"
#include <stdint.h>

/// Power Detector
/// AD8318 - 1 MHz to 8 GHz, 70 dB Logarithmic Detector/Controller

namespace ad8318
{

using namespace si;

using CorrectionValues = KValues3rdOrder<float>;

struct Converter3rdOrder
{
  explicit Converter3rdOrder(const CorrectionValues &kValues, int8_t attenuationDb = 0);

  /// Retrieve the actual k-values.
  /// \param[out] kValues
  void getCorrectionCoefficients(CorrectionValues &kValues) const;

  /// Set the k-values for upcoming conversions.
  /// \param kValues
  void setCorrectionCoefficients(const CorrectionValues &kValues);

  void   setAttenuationDb(int8_t newAttenuationDb);
  int8_t getAttenuationDb() const;

  /// Converts the raw value to dBmW according to the correction coefficients (k-values) and attenuation offset.
  /// @param rawValue the value to convert
  /// @param correctedDbmW converted and corrected value
  void convertDbMilliWatt(uint16_t rawValue, float &correctedDbmW) const;

  /// Converts the dBmW value to Watts using SI units.
  /// \param[in] correctedDbmW the dBmW value to convert to W
  /// \param[out] watt converted W value
  /// \param[out] siUnit respective SI unit
  constexpr void convertToWatt(const float &correctedDbmW, float &watt, SiUnit &siUnit);

protected:

  CorrectionValues correction{ 0, 0, 0 };   /// 3rd-order correction values.
  int8_t           attenuationDb{ 0 };
};

}   // namespace ad8318
