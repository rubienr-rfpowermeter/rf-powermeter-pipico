#pragma once

#include <stdint.h>

namespace ad7887
{

/// Power Management Bits. These two bits decode the mode of operation of the AD7887.
///
/// Mode 1 - shutdown/full power depending on CS
/// PM1=0 PM0=0
/// In this mode, the AD7887 enters shutdown if the CS input is 1 and is in full power mode when CS is 0.
/// Thus the part comes out of shutdown on the falling edge of CS and enters shutdown on the rising edge of CS.
///
/// Mode 2 - always on
/// PM1=0 PM0=1
/// In this mode, the AD7887 is always fully powered up, regardless of the status of any of the logic inputs.
///
/// Mode 3 - auto shutdown
/// PM1=1 PM0=0
/// In this mode, the AD7887 automatically enters shutdown mode at the end of each conversion, regardless of the state of CS.
///
/// Mode 4 - standby, similar to 3 but faster power up
/// PM1=1 PM0=1
/// In this standby mode, portions of the AD7887 are powered down but the on-chip reference voltage remains powered up.
/// This mode is similar to Mode 3, but allows the part to power up much faster.
/// The REF bit should be 0 to ensure that the on-chip reference is enabled.
enum ControlRegisterPowerMode : uint8_t
{
  ShutdownOnCs = 0b00,   /// AD7887 enters shutdown if the CS input is 1 and is in full power mode when CS is 0.
  AlwaysOn     = 0b01,   /// AD7887 is always fully powered up, regardless of the status of any of the logic inputs.
  AutoShutdown = 0b10,   /// AD7887 automatically enters shutdown mode at the end of each conversion,
  Standby      = 0b11,   /// Portions of the AD7887 are powered down but the on-chip reference voltage remains powered up.
};

/// CH - Channel Bit.
/// When the part is selected for dual-channel mode, this bit determines which channel is converted for the next conversion.
/// A 0 in this bit selects the AIN0 input, and a 1 in this bit selects the AIN1 input.
/// In single- channel mode, this bit should always be 0.
enum ControlRegisterChannelSelect : uint8_t
{
  AIn0 = 0,
  AIn1 = 1
};

/// SIN/DUAL - Single/Dual Bit.
/// This bit determines whether the AD7887 operates in single-channel or dual-channel mode.
/// A 0 in this bit selects single-channel operation and the AIN1/VREF pin assumes its VREF function.
/// A 1 in this bit selects dual-channel mode, with the reference voltage for the ADC internally connected to VDD and the
/// AIN1/VREF pin assuming its AIN1 function as the second analog input channel.
/// To obtain best performance from the AD7887, the internal reference should be disabled when operating in the dual-channel mode,
/// that is, REF = 1.
enum ControlRegisterChannelMode : uint8_t
{
  Single = 0,
  Dual   = 1
};

/// REF - Reference Bit.
/// With a 0 in this bit, the on-chip reference is enabled.
/// With a 1 in this bit, the on-chip reference is disabled.
/// With external V_ref on channel two this bit should be 1.
enum ControlRegisterVReference : uint8_t
{
  OnChip  = 0,
  OffChip = 1
};

struct ControlRegister8b
{
  uint8_t powerManagement        : 2 { ControlRegisterPowerMode::AlwaysOn };
  uint8_t mustBeZero1            : 1 { 0 };   /// ZERO A 0 must be written to this bit to ensure correct operation of the AD7887.
  uint8_t channelSelect          : 1 { ControlRegisterChannelSelect::AIn0 };
  uint8_t singleDualChanelSelect : 1 { ControlRegisterChannelMode::Single };
  uint8_t onChipReference        : 1 { ControlRegisterVReference::OffChip };
  uint8_t mustBeZero2            : 1 { 0 };   /// ZERO A 0 must be written to this bit to ensure correct operation of the AD7887.
  uint8_t dontCare               : 1 { 1 };   /// DONTC The value written to this bit of the control register doesn't matter.
};

struct ControlRegister16b
{
  uint8_t           zero{ 0 };
  ControlRegister8b controlRegister8b{};
};

union TransmissionData16b
{
  // TransmissionData(const ControlRegister &other);

  uint8_t            asUint8;
  uint16_t           asUint16;
  ControlRegister16b asControlRegister16b{};
};

/// Bit/Reception order:
/// - LSB of SampleRegister corresponds to MSB of reception from AT7887
/// - MSB of SampleRegister corresponds to LSB of reception from AT7887
/// - LSB of SampleRegister is received first, MSB last.

struct SampleRegister16b
{
  uint16_t raw12b : 12 { 0 };
  uint16_t zero   : 4 { 0 };
};

union ReceptionData16b
{
  SampleRegister16b asSampleRegister16b;
  uint16_t          asUint16{ 0 };
};

}   // namespace ad7887
