#pragma once

#include <initializer_list>

namespace pangolin
{

template <class Derived, class Base>
concept DerivedFrom = std::is_base_of_v<Base, Derived>;

template <class T, class U>
concept SameAs = std::is_same_v<T, U>;

template <class T>
concept EnumType = std::is_enum_v<T>;

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept Iterable = requires(T x)
{
  *std::begin(x);
  *std::end(x);
};

template <typename T>
concept Dereferencable = requires(T x)
{
  *x;
};

}  // namespace pangolin
