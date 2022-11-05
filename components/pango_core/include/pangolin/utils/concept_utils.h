#pragma once

#include <initializer_list>

namespace pangolin
{

template<class Derived, class Base>
concept DerivedFrom = std::is_base_of_v<Base, Derived>;

}
