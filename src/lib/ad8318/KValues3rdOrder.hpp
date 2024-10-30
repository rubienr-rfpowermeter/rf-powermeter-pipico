#pragma once

namespace ad8318
{

template <typename T> struct KValues3rdOrder
{
  T k0{ 1 }; /// default: no correction
  T k1{ 0 }; /// default: no correction
  T k2{ 0 }; /// default: no correction
  T k3{ 0 }; /// default: no correction
};

}   // namespace ad8318
