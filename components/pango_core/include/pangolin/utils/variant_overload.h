#pragma once

#include <variant>

namespace pangolin
{
    // Pattern from https://en.cppreference.com/w/cpp/utility/variant/visit
    template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
    template<class... Ts> overload(Ts...) -> overload<Ts...>; // for pre C++20

    // e.g.
    // std::variant<int,float,std::string,std::tuple<int,int>> v;
    // std::visit(overload {
    //         [](int arg) { },
    //         [](const std::string& arg) { }
    //         [](auto arg) { ... everything else... },
    //     }, v);
}
