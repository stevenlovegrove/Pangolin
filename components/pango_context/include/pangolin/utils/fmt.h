#include <fmt/format.h>
#include <iostream>

namespace pangolin
{

template<typename ...TArgs>
void println(TArgs... args)
{
    fmt::print(std::forward<TArgs>(args)...);
    std::cout << std::endl;
}

}
