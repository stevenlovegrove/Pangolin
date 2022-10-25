#pragma once

#include <algorithm>
#include <pangolin/context/context.h>
#include <pangolin/gui/panel.h>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Represents a (possibly nested) arrangement of Panels on screen
///
struct PanelGroup
{
    enum class Grouping
    {
        stacked,    // panes blended over one another
        tabbed,     // one pane shown at a time, with user selecting current
        horizontal, // panes share client area horizontally
        vertical    // panes share client area vertically
    };

    using Element = std::variant<Shared<PanelGroup>, Shared<Panel>>;
    std::vector<Element> vec;
    Grouping grouping = Grouping::horizontal;
};

////////////////////////////////////////////////////////////////////
// Define Convenience operators for building arrangements

// TODO: not sure this is worth it.
// void operator<(Shared<Context>& w, const Shared<PanelGroup>& g)
// {
//     w->setLayout(g);
// }

#define PANGO_COMMA ,

namespace detail
{
Shared<PanelGroup> join( PanelGroup::Grouping op_type, const Shared<PanelGroup>& lhs, const Shared<PanelGroup>& rhs ) {
    auto ret = Shared<PanelGroup>::make();
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
template<std::derived_from<Panel> T>
Shared<PanelGroup> join( PanelGroup::Grouping op_type, const Shared<PanelGroup>& lhs, const Shared<T>& rhs ) {
    auto ret = Shared<PanelGroup>::make();
    ret->grouping = op_type;
    if(op_type == lhs->grouping ) {
        ret->vec = lhs->vec;
    }else{
        ret->vec.push_back(lhs);
    }
    ret->vec.push_back(rhs);
    return ret;
}
template<std::derived_from<Panel> T>
Shared<PanelGroup> join( PanelGroup::Grouping op_type, const Shared<T>& lhs, const Shared<PanelGroup>& rhs ) {
    auto ret = Shared<PanelGroup>::make();
    ret->grouping = op_type;
    ret->vec.push_back(lhs);
    if(op_type == rhs->grouping) {
        ret->vec.insert(ret->vec.end(), rhs->vec.begin(), rhs->vec.end());
    }else{
        ret->vec.push_back(rhs);
    }
    return ret;
}
template<std::derived_from<Panel> LHS, std::derived_from<Panel> RHS>
Shared<PanelGroup> join( PanelGroup::Grouping op_type, const Shared<LHS>& lhs, const Shared<RHS>& rhs ) {
    auto ret = Shared<PanelGroup>::make();
    ret->grouping = op_type;
    ret->vec.push_back(lhs);
    ret->vec.push_back(rhs);
    return ret;
}
}

#define PANGO_PANEL_OPERATOR(op, op_type) \
    Shared<PanelGroup> op(const Shared<PanelGroup>& lhs, const Shared<PanelGroup>& rhs) { \
        return detail::join(op_type, lhs, rhs); \
    } \
    template<std::derived_from<Panel> T> \
    Shared<PanelGroup> op(const Shared<PanelGroup>& lhs, const Shared<T>& rhs) { \
        return detail::join(op_type, lhs, rhs); \
    } \
    template<std::derived_from<Panel> T> \
    Shared<PanelGroup> op(const Shared<T>& lhs, const Shared<PanelGroup>& rhs) { \
        return detail::join(op_type, lhs, rhs); \
    } \
    template<std::derived_from<Panel> T1, std::derived_from<Panel> T2> \
    Shared<PanelGroup> op(Shared<T1>& lhs, Shared<T2>& rhs)  { \
        return detail::join(op_type, lhs, rhs); \
    }

PANGO_PANEL_OPERATOR(operator PANGO_COMMA, PanelGroup::Grouping::tabbed)
PANGO_PANEL_OPERATOR(operator|, PanelGroup::Grouping::horizontal)
PANGO_PANEL_OPERATOR(operator/, PanelGroup::Grouping::vertical)
PANGO_PANEL_OPERATOR(operator^, PanelGroup::Grouping::stacked)
#undef PANGO_PANEL_OPERATOR
#undef PANGO_COMMA

}
