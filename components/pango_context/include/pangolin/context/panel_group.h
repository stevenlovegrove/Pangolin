#pragma once

#include "common.h"
#include "panel.h"

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Represents a (possibly nested) arrangement of Panels on screen
///
struct PanelGroup
{
    enum class Type
    {
        stacked,    // panes blended over one another
        tabbed,     // one pane shown at a time, with user selecting current
        horizontal, // panes share client area horizontally
        vertical    // panes share client area vertically
    };

    using Element = std::variant<Shared<PanelGroup>, Shared<Panel>>;
    std::vector<Element> elements_;
    Type group_type_;
};

////////////////////////////////////////////////////////////////////
// Define Convenience operators for building arrangements

void operator<(const Shared<Window>& w, const Shared<PanelGroup>& g)
{
}

#define PANGO_COMMA ,

#define PANGO_PANEL_OPERATOR(op, type) \
    Shared<PanelGroup> op(const Shared<PanelGroup>& g1, const Shared<PanelGroup>& g2) \
    { \
        return g1; \
    } \
 \
    template<std::derived_from<Panel> T> \
    Shared<PanelGroup> op(const Shared<PanelGroup>& group, Shared<T>& p2) \
    { \
        return group; \
    } \
 \
    template<std::derived_from<Panel> T> \
    Shared<PanelGroup> op(Shared<T>& p2, const Shared<PanelGroup>& group) \
    { \
        return group; \
    } \
 \
    template<std::derived_from<Panel> T1, std::derived_from<Panel> T2> \
    Shared<PanelGroup> op(Shared<T1>& p1, Shared<T2>& p2) \
    { \
        return Shared<PanelGroup>::make(); \
    }

PANGO_PANEL_OPERATOR(operator PANGO_COMMA, tabbed)
PANGO_PANEL_OPERATOR(operator|, tabbed)
PANGO_PANEL_OPERATOR(operator/, vertical)
PANGO_PANEL_OPERATOR(operator^, stacked)
#undef PANGO_PANEL_OPERATOR
#undef PANGO_COMMA

}