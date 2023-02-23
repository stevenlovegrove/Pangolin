#pragma once

#include <pangolin/utils/concept_utils.h>

#include <functional>

namespace pangolin
{

template <typename T, Iterable Container>
requires Dereferencable<typename Container::value_type>
void forAllT(Container& container, const std::function<void(T&)>& user_func)
{
  for (auto& x : container) {
    T* maybe_ptr = dynamic_cast<T*>(&(*x));
    if (maybe_ptr) user_func(*maybe_ptr);
  }
}

}  // namespace pangolin
