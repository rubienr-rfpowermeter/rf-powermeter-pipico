#pragma once

#include "lib/ad7887/registers.h"
#include "lib/sample_data/Average.h"
#include "lib/si/si_types.h"
#include "modules/rf_power_meter/converter_types.h"

using AveragedSiFloat  = ResultT<si::SiFloat>;
using AveragedSiUint16 = ResultT<si::SiUint16>;
using AveragedSiInt8   = ResultT<si::SiInt8>;

struct Temperature
{
  AveragedSiFloat volt{};
  AveragedSiFloat kelvin{};
  AveragedSiFloat celsius{};
};

struct ConvertedSample
{
  AveragedSiFloat value_dbv{};
  AveragedSiFloat value_linearv{};
};

struct TransactionData
{
  uint32_t               timestamp_us{ 0 };
  Temperature            probe_temperature{};
  rfpm::CorrectionValues correction_values{};
  rfpm::FrequencyBand    frequency_band{};
  ConvertedSample        converted_sample{};
};
