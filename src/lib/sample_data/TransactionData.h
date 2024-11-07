#pragma once

#include "lib/ad7887/registers.h"
#include "lib/sample_data/Average.h"
#include "modules/rf_power_meter/converter_types.h"

using SiInt8   = si::Value<uint8_t>;
using SiUint16 = si::Value<uint16_t>;
using SiFloat  = si::Value<float>;

using AveragedSiFloat  = ResultT<SiFloat>;
using AveragedSiUint16 = ResultT<SiUint16>;
using AveragedSiInt8   = ResultT<SiInt8>;

struct Temperature
{
  AveragedSiUint16 volt_em4{};
  AveragedSiUint16 kelvin_em2{};
  AveragedSiInt8   celsius_em2{};
};

struct ConvertedSample
{
  AveragedSiFloat value_dbv{};
  AveragedSiFloat value_linearv{};
};

struct TransactionData
{
  uint32_t        timestamp_ms{ 0 };
  Temperature     probe_temperature{};
  ConvertedSample converted_sample{};
};
