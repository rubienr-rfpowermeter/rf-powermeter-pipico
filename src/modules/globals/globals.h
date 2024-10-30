#pragma once

#include "lib/ad7887/AD7887.h"
#include "lib/ad7887/registers.h"
#include "lib/ad8318/AD8318.h"
#include "lib/ad8318/KValues3rdOrder.hpp"
#include "lib/average/Average.h"
#include <stdint.h>

using namespace ad7887;
using namespace ad8318;
using namespace avg;


typedef struct Probe
{
  ControlRegister ctlRegister{ .powerManagement0       = 1,
                               .powerManagement1       = 0,
                               .mustBeZero1            = 0,
                               .channelSelect          = 0,
                               .singleDualChanelSelect = 0,
                               .onChipReference        = 1,
                               .mustBeZero2            = 0,
                               .dontCare               = 0 };
  // AD7887 device{ ctlRegister, chipSelectDigitalWrite, clkDigitalWrite, dataDigitalWrite, dataDigitalRead, delayMicroseconds };
  ad7887::SampleRegister sampleRegister{ .raw12Bit = 0, .zero = 0 };
  avg::Average32x16 sampleAverage;
  KValues3rdOrderFloat kValues{};
  AD8318Converter3rdOrder converter{ kValues };

  struct
  {
    uint16_t rawSample10Bit{ 0 };
    uint16_t volt_em4{ 0 };
    uint16_t kelvin_em2{ 0 };
    int16_t celsius_em2{ 0 };
  } temperature;
} Probe;

typedef struct Globals
{
  Probe probe{};
} Globals;

extern Globals globals;
