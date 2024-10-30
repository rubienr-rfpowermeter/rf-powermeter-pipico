#pragma once
#include <stdint.h>

#include <cmath>
#include <math.h>
#include <utility>

namespace si
{

enum class SiUnit : uint8_t
{
  Kilo = 0,   /// 10^3
  TimesOne,   /// 10^0
  Milli,      /// 10^-3
  Micro,      /// 10^-6
  Nano,       /// 10^-9
  Pico,       /// 10^-12
  Femto,      /// 10^-15
  Undefined
};

constexpr char siUnitToChar(const SiUnit t)
{
  switch (t)
  {
  case SiUnit::Kilo:
    return 'k';
  case SiUnit::TimesOne:
    return ' ';
  case SiUnit::Milli:
    return 'm';
  case SiUnit::Micro:
    return 'u';
  case SiUnit::Nano:
    return 'n';
  case SiUnit::Pico:
    return 'p';
  case SiUnit::Femto:
    return 'f';
  default:
    return '?';
  }
}

constexpr uint8_t siUnitToUnderlyingType(SiUnit u) { return std::to_underlying(u); }

constexpr SiUnit siUnitFromUnderlyingType(uint8_t u)
{
  return (static_cast<uint8_t>(SiUnit::Kilo) <= u && u < static_cast<uint8_t>(SiUnit::Undefined)) ? static_cast<SiUnit>(u) : SiUnit::Undefined;
}

constexpr float SI_FACTORS[] = {
  [siUnitToUnderlyingType(SiUnit::Kilo)]      = 1000,                /// 10^3
  [siUnitToUnderlyingType(SiUnit::TimesOne)]  = 1,                   /// 10^0
  [siUnitToUnderlyingType(SiUnit::Milli)]     = 0.001,               /// 10^-3
  [siUnitToUnderlyingType(SiUnit::Micro)]     = 0.000001,            /// 10^-6
  [siUnitToUnderlyingType(SiUnit::Nano)]      = 0.000000001,         /// 10^-9
  [siUnitToUnderlyingType(SiUnit::Pico)]      = 0.000000000001,      /// 10^-12
  [siUnitToUnderlyingType(SiUnit::Femto)]     = 0.000000000000001,   /// 10^-15
  [siUnitToUnderlyingType(SiUnit::Undefined)] = 0,
};

}   // namespace si
