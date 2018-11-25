#pragma once

#include <type_traits>
#include <utility>
#include <iostream>
#include <sstream>

namespace pangolin {

// Provide SFINAE test for if type T is streamable into S
// Example usage:
//    template <typename S>
//    typename std::enable_if<is_streamable<S>::value, S>::type
//    Example(const S& src) noexcept
//    {
//        std::cout << src;
//        return src;
//    }
template<typename T, typename S = std::ostream>
class is_streamable
{
    template<typename SS, typename TT>
    static auto test(int)
    -> decltype( std::declval<SS&>() << std::declval<TT>(), std::true_type() );

    template<typename, typename>
    static auto test(...) -> std::false_type;

public:
    static const bool value = decltype(test<S,T>(0))::value;
};



}
