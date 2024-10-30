#pragma once

#pragma once

#include "lib/ad7887/registers.h"
#include "lib/ad8318/Converter3rdOrder.h"
#include "lib/ad8318/KValues3rdOrder.hpp"
#include "lib/sample_data/Average.h"
#include <stdint.h>

using namespace ad7887;
using namespace ad8318;

struct Temperature
{
  uint16_t rawSample10Bit{ 0 };
  uint16_t volt_em4{ 0 };
  uint16_t kelvin_em2{ 0 };
  int16_t  celsius_em2{ 0 };
};

struct Probe
{
  /*
  ControlRegister ctlRegister{ .powerManagement0       = 1,
                               .powerManagement1       = 0,
                               .mustBeZero1            = 0,
                               .channelSelect          = 0,
                               .singleDualChanelSelect = 0,
                               .onChipReference        = 1,
                               .mustBeZero2            = 0,
                               .dontCare               = 0 };
  AD7887 device{ ctlRegister, chipSelectDigitalWrite, clkDigitalWrite, dataDigitalWrite, dataDigitalRead, delayMicroseconds };
  ad7887::SampleRegister sampleRegister{};
  */
  Temperature temperature{};
};

struct TransactionData
{
  uint32_t timestamp_ms;
  Probe    probe{};
  Sample   raw_sample{};
};
