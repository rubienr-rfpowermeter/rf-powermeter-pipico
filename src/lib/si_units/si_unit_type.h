#pragma once
#include <stdint.h>

namespace si
{

enum class UnitType : uint8_t
{
  Kilo = 1,   /// 10^3
  TimesOne,   /// 10^0
  Milli,      /// 10^-3
  Micro,      /// 10^-6
  Nano,       /// 10^-9
  Pico,       /// 10^-12
  Femto,      /// 10^-15
  Undefined
};

char unitTypeToChar(UnitType const t);

constexpr uint8_t unitTypeToUnderlyingType(UnitType u) { return static_cast<uint8_t>(u); }

constexpr UnitType unitTypeFromUnderlyingType(uint8_t u)
{
  return (static_cast<uint8_t>(UnitType::Kilo) <= u && u <= static_cast<uint8_t>(UnitType::Undefined)) ? static_cast<UnitType>(u) :
                                                                                                         UnitType::Undefined;
}

}   // namespace si
