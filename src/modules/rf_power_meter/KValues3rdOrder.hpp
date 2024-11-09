#pragma once

namespace rfpm
{

template <typename T> struct KValues3rdOrder
{
  T k0{ 0 };   /// default: no correction
  T k1{ 0 };   /// default: no correction
  T k2{ 0 };   /// default: no correction
  T k3{ 0 };   /// default: no correction
};

}   // namespace rfpm
