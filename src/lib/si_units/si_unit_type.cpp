#include "si_unit_type.h"

namespace si
{

char unitTypeToChar(const UnitType t)
{
  switch(t)
  {
  case UnitType::Kilo:
    return 'k';
  case UnitType::TimesOne:
    return ' ';
  case UnitType::Milli:
    return 'm';
  case UnitType::Micro:
    return 'u';
  case UnitType::Nano:
    return 'n';
  case UnitType::Pico:
    return 'p';
  case UnitType::Femto:
    return 'f';
  default:
    return '?';
  }
}

} // namespace si
