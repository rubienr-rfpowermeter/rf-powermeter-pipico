#include "AD7887.h"

namespace ad7887
{

AD7887::AD7887(
  const ControlRegister &ctrlRegister,
  void (*chipSelectDigitalWrite)(uint8_t),
  void (*clkDigitalWrite)(uint8_t),
  void (*dataDigitalWrite)(uint8_t),
  int (*dataDigitalRead)(),
  void (*delayMicroSeconds)(unsigned int)) :
receptionData(), transmissionData(ctrlRegister), clockWrite(clkDigitalWrite), chipSelectWrite(chipSelectDigitalWrite),
dataWrite(dataDigitalWrite), dataRead(dataDigitalRead), delayUs(delayMicroSeconds)
{
}

void AD7887::uint16Transaction()
{
  uint16_t currentDataBit = 0x8000;

#if defined(AD7887_T2_DELAY_US)
  delayUs(AD7887_T2_DELAY_US);
#endif
  chipSelectWrite(0);
  while (currentDataBit)
  {
    // 1. write data bit
    dataWrite((transmissionData.asUint16 & currentDataBit) ? 1 : 0);
    currentDataBit >>= 1;

    // 2. clock out (high + low)
    clockWrite(0);
#if defined(AD7887_T7_DELAY_US)
    delayUs(AD7887_T7_DELAY_US);
#endif
    clockWrite(1);
#if defined(AD7887_T6_DELAY_US)
    delayUs(AD7887_T6_DELAY_US);
#endif
    // 3. read data bit
    receptionData.asUint16 <<= 1;
    receptionData.asUint16 |= dataRead() ? 1 : 0;
  }
  chipSelectWrite(1);
}

bool AD7887::readSample(SampleRegister &sample)
{
  uint16Transaction();
  if (receptionData.asSampleRegister.zero != 0) return false;
  sample = receptionData.asSampleRegister;
  return true;
}

void AD7887::updateControlRegister(const ControlRegister &newFlags)
{
  transmissionData.asUint16ControlRegister.controlRegister = newFlags;
}

const ControlRegister &AD7887::getControlRegister() const { return transmissionData.asUint16ControlRegister.controlRegister; }

}   // namespace ad7887
