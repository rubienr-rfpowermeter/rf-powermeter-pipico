#pragma once
// #include "../rfmeter_configuration.h"
#include "registers.h"
#include <stdint.h>

/// Micro power AD converter
/// AD7887 - 2.7 V to 5.25 V, Micropower, 2-Channel, 125 kSPS, 12-Bit ADC in 8-Lead MSOP

namespace ad7887
{

struct AD7887
{
  AD7887(
    const ControlRegister &ctrlRegister,
    void (*chipSelectDigitalWrite)(uint8_t),
    void (*clkDigitalWrite)(uint8_t),
    void (*dataDigitalWrite)(uint8_t),
    int (*dataDigitalRead)(),
    void (*delayMicroSeconds)(unsigned int));

  /// Performs a "transaction": sending control flags and reading data in one run.
  /// @param sample overwritten by received data if true was returned, untouched otherwise
  /// @return false if expected zero-flags in received data are not 0
  bool readSample(SampleRegister &sample);

  /// Updates the control register which is transmitted on each subsequent transaction.
  /// Note that the actually sent control flags are not effective on the current transaction (but afterwards).
  /// @param newFlags
  void updateControlRegister(const ControlRegister &newFlags);

  [[nodiscard]]
  const ControlRegister &getControlRegister() const;

  protected:
  void uint16Transaction();

  ReceptionData receptionData;
  TransmissionData transmissionData;

  void (*clockWrite)(uint8_t);
  void (*chipSelectWrite)(uint8_t);
  void (*dataWrite)(uint8_t);
  int (*dataRead)();
  void (*delayUs)(unsigned int);
};

} // namespace ad7887
