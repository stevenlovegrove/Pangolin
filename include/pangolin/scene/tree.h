#pragma once

#include <map>
#include <pangolin/compat/optional.h>

namespace pangolin {

template<typename T, typename TEdgeKey, typename TEdge>
struct TreeNode
{
    T item;
    std::map<TEdgeKey,TEdge> edges;
};

struct Renderable
{
    virtual ~Renderable() = 0;
    virtual void PreRender() = 0;
    virtual void DoRender() = 0;
    virtual void PostRender() = 0;
};


}
