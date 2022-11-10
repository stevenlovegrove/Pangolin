#pragma once

#include <initializer_list>

namespace pangolin
{

template<class Derived, class Base>
concept DerivedFrom = std::is_base_of_v<Base, Derived>;

template< class T, class U >
concept SameAs = std::is_same_v<T, U>;


}
