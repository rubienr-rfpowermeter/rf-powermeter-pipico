#pragma once

#include <cinttypes>
#include <cmath>
#include <string>
#include <utility>

namespace si
{

enum struct Unit : uint8_t
{
  Undefined = 0,
  Celsius,
  Hertz,
  Kelvin,
  NoUnit,
  Volt,
  Watt,
  TotalUnits
};

enum struct Scale : uint8_t
{
  Undefined = 0,
  Giga,       /// 10^9
  Mega,       /// 10^6
  Kilo,       /// 10^3
  TimesOne,   /// 10^0
  Milli,      /// 10^-3
  Micro,      /// 10^-6
  Nano,       /// 10^-9
  Pico,       /// 10^-12
  Femto,      /// 10^-15
  TotalScales
};

enum struct Linearity : uint8_t
{
  Undefined = 0,
  Linear,
  Db10,
  Db20,
  TotalLinearities
};

constexpr const char *linearityToStr(const Linearity t)
{
  switch (t)
  {
  case Linearity::Linear:
    return " ";
  case Linearity::Db10:
    return "dB";
  case Linearity::Db20:
    return "dB";
  default:
    return "?";
  }
}

constexpr const char *unitToStr(const Unit t)
{
  switch (t)
  {
  case Unit::Celsius:
    return "°C";
  case Unit::Hertz:
    return "Hz";
  case Unit::Kelvin:
    return "°K";
  case Unit::NoUnit:
    return "";
  case Unit::Volt:
    return "V";
  case Unit::Watt:
    return "W";
  default:
    return "?";
  }
}

constexpr const char *scaleToStr(const Scale t)
{
  switch (t)
  {
  case Scale::Giga:
    return "G";
  case Scale::Mega:
    return "M";
  case Scale::Kilo:
    return "k";
  case Scale::TimesOne:
    return "";
  case Scale::Milli:
    return "m";
  case Scale::Micro:
    return "u";
  case Scale::Nano:
    return "n";
  case Scale::Pico:
    return "p";
  case Scale::Femto:
    return "f";
  default:
    return "?";
  }
}

template <typename T> struct Value
{
  T         value{ std::numeric_limits<T>::quiet_NaN() };
  Linearity lin{ Linearity::Linear };
  Scale     scale{ Scale::TimesOne };
  Unit      unit{ Unit::NoUnit };
};

constexpr uint8_t linearityToUnderlyingType(Linearity u) { return std::to_underlying<Linearity>(u); }

constexpr uint8_t unitToUnderlyingType(Unit u) { return std::to_underlying<Unit>(u); }

constexpr uint8_t scaleToUnderlyingType(Scale u) { return std::to_underlying<Scale>(u); }

constexpr Linearity linearityFromUnderlyingType(uint8_t u)
{
  return (static_cast<uint8_t>(Linearity::Undefined) <= u && u < static_cast<uint8_t>(Linearity::TotalLinearities)) ?
           static_cast<Linearity>(u) :
           Linearity::Undefined;
}

constexpr Unit unitFromUnderlyingType(uint8_t u)
{
  return (static_cast<uint8_t>(Unit::Undefined) <= u && u < static_cast<uint8_t>(Unit::TotalUnits)) ? static_cast<Unit>(u) : Unit::Undefined;
}

constexpr Scale scaleFromUnderlyingType(uint8_t u)
{
  return (static_cast<uint8_t>(Scale::Undefined) <= u && u < static_cast<uint8_t>(Scale::TotalScales)) ? static_cast<Scale>(u) :
                                                                                                         Scale::Undefined;
}

constexpr float SI_FACTORS[std::to_underlying<Scale>(Scale::TotalScales)] = {
  [scaleToUnderlyingType(Scale::Undefined)] = 0,
  [scaleToUnderlyingType(Scale::Giga)]      = 1000000000.0f,      /// 10^9
  [scaleToUnderlyingType(Scale::Mega)]      = 1000000.0f,         /// 10^6
  [scaleToUnderlyingType(Scale::Kilo)]      = 1000.0f,            /// 10^3
  [scaleToUnderlyingType(Scale::TimesOne)]  = 1.0f,               /// 10^0
  [scaleToUnderlyingType(Scale::Milli)]     = 0.001,              /// 10^-3
  [scaleToUnderlyingType(Scale::Micro)]     = 0.000001,           /// 10^-6
  [scaleToUnderlyingType(Scale::Nano)]      = 0.000000001,        /// 10^-9
  [scaleToUnderlyingType(Scale::Pico)]      = 0.000000000001,     /// 10^-12
  [scaleToUnderlyingType(Scale::Femto)]     = 0.000000000000001   /// 10^-15
};

}   // namespace si
