#include "AD8318.h"
#include <math.h>

namespace ad8318
{

Converter3rdOrder::Converter3rdOrder(const CorrectionValues &kValues, int8_t attenuationDb) :
correction(kValues), attenuationDb(attenuationDb)
{
}

void Converter3rdOrder::getCorrectionCoefficients(CorrectionValues &kValues) const { kValues = correction; }

void Converter3rdOrder::setCorrectionCoefficients(const CorrectionValues &kValues) { correction = kValues; }

void Converter3rdOrder::setAttenuationDb(int8_t newAttenuationDb) { attenuationDb = newAttenuationDb; }

int8_t Converter3rdOrder::getAttenuationDb() const { return attenuationDb; }

void Converter3rdOrder::convertDbMilliWatt(uint16_t rawValue, float &correctedDbmW) const
{
  const float v{ static_cast<float>(rawValue) };
  const float v2 = { v * v };
  const float v3 = { v2 * v };
  correctedDbmW  = correction.k0 + correction.k1 * v + correction.k2 * v2 + correction.k3 * v3;
}

constexpr void Converter3rdOrder::convertToWatt(const float &correctedDbmW, float &watt, SiUnit &siUnit)
{
  const float milliWatt = powf(10.0f, correctedDbmW / 10.0f);

  if (milliWatt < 0.000000001f)   // femto Watt
  {
    watt   = milliWatt * 1e12f;
    siUnit = SiUnit::Femto;
  }
  else if (milliWatt < 0.000001f)   // pico Watt
  {
    watt   = milliWatt * 1e9f;
    siUnit = SiUnit::Pico;
  }
  else if (watt < 0.001f)   // nano Watt
  {
    watt   = milliWatt * 1e6f;
    siUnit = SiUnit::Nano;
  }
  else if (watt < 1.0f)   // micro Watt
  {
    watt   = milliWatt * 1e3f;
    siUnit = SiUnit::Micro;
  }
  else if (milliWatt < 1000.0f)   // milli Watt
  {
    // milliWatt *= 10e0f;
    siUnit = SiUnit::Milli;
  }
  else   // Watt
  {
    watt   = milliWatt * 10e-3f;
    siUnit = SiUnit::TimesOne;
  }
}

}   // namespace ad8318
