#include "Converter3rdOrder.h"
#include <math.h>

namespace ad8318
{

Converter3rdOrder::Converter3rdOrder(const CorrectionValues &kValues, float attenuationDb) :
correction(kValues), attenuationDb(attenuationDb)
{
}

void Converter3rdOrder::getCorrectionCoefficients(CorrectionValues &kValues) const { kValues = correction; }

void Converter3rdOrder::setCorrectionCoefficients(const CorrectionValues &kValues) { correction = kValues; }

void Converter3rdOrder::setAttenuationDb(float newAttenuationDb) { attenuationDb = newAttenuationDb; }

float Converter3rdOrder::getAttenuationDb() const { return attenuationDb; }

float Converter3rdOrder::toCorrectedDbmW(uint16_t uncorrectedDbmW) const
{
  const float v{ static_cast<float>(uncorrectedDbmW) };
  const float v2 = { v * v };
  const float v3 = { v2 * v };
  return attenuationDb + correction.k0 + correction.k1 * v + correction.k2 * v2 + correction.k3 * v3;
}

}   // namespace ad8318
