#pragma once
#include "../si_units/si_unit_type.h"
#include "KValues3rdOrder.hpp"
#include <stdint.h>

/// Power Detector
/// AD8318 - 1 MHz to 8 GHz, 70 dB Logarithmic Detector/Controller

namespace ad8318
{

using namespace si;

using KValues3rdOrderFloat = KValues3rdOrder<float>;

struct AD8318Converter3rdOrder
{
  explicit AD8318Converter3rdOrder(const KValues3rdOrderFloat &kValues, uint8_t attenuationDb = 0);

  /// Retrieve the actual k-values.
  /// \param[out] kValues
  void getCorrectionCoefficients(KValues3rdOrderFloat &kValues);

  /// Set the k-values for upcoming conversions.
  /// \param kValues
  void setCorrectionCoefficients(const KValues3rdOrderFloat &kValues);

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
  void convertWatt(const float &correctedDbmW, float &watt, UnitType &siUnit);

protected:

  /// @{
  /// 3rd-order correction values.
  float k0{ 0 };
  float k1{ 0 };
  float k2{ 0 };
  float k3{ 0 };
  /// @}

  int8_t attenuationDb{ 0 };
};

}   // namespace ad8318
