#pragma once

#include "common.h"

// TODO: Not sure about this yet...

namespace pangolin
{

// Default constructable wrapper for pangolin Create pattern
template<typename T>
struct Obj : public std::shared_ptr<T>
{
    Obj(const typename T::Params& p)
        : std::shared_ptr<T>(T::Create(p))
    {
    }

    Obj()
        : std::shared_ptr<T>(T::Create({}))
    {
    }
};

// e.g.
// std::vector<Obj<MultiPanel>> lots_of_panels =  {
//     {{.title="1"}}, {{.title="2"}}, {{.title="3"}}, 
// };

}