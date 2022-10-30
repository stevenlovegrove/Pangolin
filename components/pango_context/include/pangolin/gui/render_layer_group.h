#pragma once

#include <algorithm>
#include <pangolin/context/context.h>
#include <pangolin/gui/render_layer.h>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Represents a (possibly nested) arrangement of Panels on screen
///
struct RenderLayerGroup
{
    enum class Grouping
    {
        stacked,    // panes blended over one another
        tabbed,     // one pane shown at a time, with user selecting current
        horizontal, // panes share client area horizontally
        vertical    // panes share client area vertically
    };

    using Element = std::variant<Shared<RenderLayerGroup>, Shared<RenderLayer>>;
    std::vector<Element> vec;
    Grouping grouping = Grouping::horizontal;
};

////////////////////////////////////////////////////////////////////
// Define Convenience operators for building arrangements

// TODO: not sure this is worth it.
// void operator<(Shared<Context>& w, const Shared<RenderLayerGroup>& g)
// {
//     w->setLayout(g);
// }

#define PANGO_COMMA ,

namespace detail
{
Shared<RenderLayerGroup> join( RenderLayerGroup::Grouping op_type, const Shared<RenderLayerGroup>& lhs, const Shared<RenderLayerGroup>& rhs ) {
    auto ret = Shared<RenderLayerGroup>::make();
    ret->grouping = op_type;
    if(op_type == lhs->grouping && op_type == rhs->grouping ) {
        ret->vec = lhs->vec;
        ret->vec.insert(ret->vec.end(), rhs->vec.begin(), rhs->vec.end());
    }else{
        ret->vec.push_back(lhs);
        ret->vec.push_back(rhs);
    }
    return ret;
}
template<std::derived_from<RenderLayer> T>
Shared<RenderLayerGroup> join( RenderLayerGroup::Grouping op_type, const Shared<RenderLayerGroup>& lhs, const Shared<T>& rhs ) {
    auto ret = Shared<RenderLayerGroup>::make();
    ret->grouping = op_type;
    if(op_type == lhs->grouping ) {
        ret->vec = lhs->vec;
    }else{
        ret->vec.push_back(lhs);
    }
    ret->vec.push_back(rhs);
    return ret;
}
template<std::derived_from<RenderLayer> T>
Shared<RenderLayerGroup> join( RenderLayerGroup::Grouping op_type, const Shared<T>& lhs, const Shared<RenderLayerGroup>& rhs ) {
    auto ret = Shared<RenderLayerGroup>::make();
    ret->grouping = op_type;
    ret->vec.push_back(lhs);
    if(op_type == rhs->grouping) {
        ret->vec.insert(ret->vec.end(), rhs->vec.begin(), rhs->vec.end());
    }else{
        ret->vec.push_back(rhs);
    }
    return ret;
}
template<std::derived_from<RenderLayer> LHS, std::derived_from<RenderLayer> RHS>
Shared<RenderLayerGroup> join( RenderLayerGroup::Grouping op_type, const Shared<LHS>& lhs, const Shared<RHS>& rhs ) {
    auto ret = Shared<RenderLayerGroup>::make();
    ret->grouping = op_type;
    ret->vec.push_back(lhs);
    ret->vec.push_back(rhs);
    return ret;
}
}

#define PANGO_PANEL_OPERATOR(op, op_type) \
    Shared<RenderLayerGroup> op(const Shared<RenderLayerGroup>& lhs, const Shared<RenderLayerGroup>& rhs) { \
        return detail::join(op_type, lhs, rhs); \
    } \
    template<std::derived_from<RenderLayer> T> \
    Shared<RenderLayerGroup> op(const Shared<RenderLayerGroup>& lhs, const Shared<T>& rhs) { \
        return detail::join(op_type, lhs, rhs); \
    } \
    template<std::derived_from<RenderLayer> T> \
    Shared<RenderLayerGroup> op(const Shared<T>& lhs, const Shared<RenderLayerGroup>& rhs) { \
        return detail::join(op_type, lhs, rhs); \
    } \
    template<std::derived_from<RenderLayer> T1, std::derived_from<RenderLayer> T2> \
    Shared<RenderLayerGroup> op(Shared<T1>& lhs, Shared<T2>& rhs)  { \
        return detail::join(op_type, lhs, rhs); \
    }

PANGO_PANEL_OPERATOR(operator PANGO_COMMA, RenderLayerGroup::Grouping::tabbed)
PANGO_PANEL_OPERATOR(operator|, RenderLayerGroup::Grouping::horizontal)
PANGO_PANEL_OPERATOR(operator/, RenderLayerGroup::Grouping::vertical)
PANGO_PANEL_OPERATOR(operator^, RenderLayerGroup::Grouping::stacked)
#undef PANGO_PANEL_OPERATOR
#undef PANGO_COMMA

}
