#pragma once

namespace pangolin
{

template <typename T>
using IteratorType = decltype(std::begin(std::declval<T>()));

template <typename From, typename To>
concept ConvertableTo = std::is_convertible_v<From, To>;

template <typename T>
concept Range = requires(T range) {
                  {
                    std::begin(range)
                    } -> ConvertableTo<IteratorType<T>>;
                  {
                    std::end(range)
                    } -> ConvertableTo<IteratorType<T>>;
                };

// https://stackoverflow.com/a/66053335

template <typename T>
struct reversion_wrapper {
  T& iterable;
};

template <typename T>
auto begin(reversion_wrapper<T> w)
{
  return std::rbegin(w.iterable);
}

template <typename T>
auto end(reversion_wrapper<T> w)
{
  return std::rend(w.iterable);
}

template <Range T>
reversion_wrapper<T> reverse(T&& iterable)
{
  return {iterable};
}

}  // namespace pangolin
