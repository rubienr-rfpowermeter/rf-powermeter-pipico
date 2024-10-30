#include "registers.h"

namespace ad7887
{

TransmissionData::TransmissionData(const ControlRegister &ctlRegister)
{
  asUint16ControlRegister.controlRegister = ctlRegister;
  asUint16ControlRegister.zero            = 0;
}

}   // namespace ad7887
