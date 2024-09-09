#include "registers.h"

namespace ad7887
{

TransmissionData::TransmissionData() : asUint16(0) { }
TransmissionData::TransmissionData(const ControlRegister &ctlRegister)
{
  asUint16ControlRegister.controlRegister = ctlRegister;
  asUint16ControlRegister.zero            = 0;
}

ReceptionData::ReceptionData() : asUint16(0) { }

} // namespace ad7887
