#pragma once

#include <tuple>

namespace pangolin
{

template <typename TPred, typename T>
bool all_of(TPred pred, const T& i)
{
  return pred(i);
}

template <typename TPred, typename T, typename... Targs>
bool all_of(TPred const& pred, const T& i, Targs const&... ins)
{
  return pred(i) && all_of(pred, ins...);
}

template <typename TPred, typename T>
bool any_of(TPred pred, const T& i)
{
  return pred(i);
}

template <typename TPred, typename T, typename... Targs>
bool any_of(TPred const& pred, const T& i, Targs&... ins)
{
  return pred(i) || any_of(pred, ins...);
}

template <typename TContainer, typename... Targs>
bool all_found(TContainer const& c, Targs const&... its)
{
  using T1 = typename std::tuple_element<0, std::tuple<Targs...>>::type;
  return all_of([&c](const T1& it) { return it != c.end(); }, its...);
}

template <typename T, typename... Targs>
bool all_equal(const T& v1, Targs const&... its)
{
  return all_of([v1](const T& o) { return v1 == o; }, its...);
}

}  // namespace pangolin
