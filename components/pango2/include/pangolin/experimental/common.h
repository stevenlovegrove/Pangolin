#pragma once

#include <type_traits>
#include <vector>
#include <memory>
#include <iostream>
#include <Eigen/Core>
#include <pangolin/experimental/shared.h>

namespace pangolin
{

// TODO: organize these into better places

template<typename T>
using shared_vector = std::vector<std::shared_ptr<T>>;

template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

}