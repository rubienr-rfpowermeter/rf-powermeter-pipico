#pragma once

#include "lib/ad7887/registers.h"
#include "lib/sample_data/Average.h"

using namespace ad7887;

struct Temperature
{
  uint16_t rawSample10Bit{ 0 };
  uint16_t volt_em4{ 0 };
  uint16_t kelvin_em2{ 0 };
  int16_t  celsius_em2{ 0 };
};

struct Probe
{
  Temperature temperature{};
};

struct TransactionData
{
  uint32_t timestamp_ms {0};
  Probe    probe{};
  Sample   raw_sample{};
};
