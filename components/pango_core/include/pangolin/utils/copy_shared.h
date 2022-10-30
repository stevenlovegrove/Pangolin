#pragma once

#include <memory>

namespace pangolin
{

template<class T>
std::shared_ptr<T> copyShared(const T& x)
{
    return std::make_shared<T>(x);
}

template<class T>
std::shared_ptr<T> moveShared(T&& x)
{
    return std::make_shared<T>(std::move(x));
}

}
