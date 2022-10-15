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

    using Element = std::variant<std::shared_ptr<PanelGroup>, std::shared_ptr<Panel>>;
    std::vector<Element> elements_;
    Type group_type_;
};

////////////////////////////////////////////////////////////////////
// Define Convenience operators for building arrangements

void operator<(const std::shared_ptr<Window>& w, const std::shared_ptr<PanelGroup>& g)
{
}

#define PANGO_COMMA ,

#define PANGO_PANEL_OPERATOR(op, type) \
    std::shared_ptr<PanelGroup> op(const std::shared_ptr<PanelGroup>& g1, const std::shared_ptr<PanelGroup>& g2) \
    { \
        return {}; \
    } \
 \
    template<Derived<Panel> T> \
    std::shared_ptr<PanelGroup> op(const std::shared_ptr<PanelGroup>& group, std::shared_ptr<T>& p2) \
    { \
        return {}; \
    } \
 \
    template<Derived<Panel> T> \
    std::shared_ptr<PanelGroup> op(std::shared_ptr<T>& p2, const std::shared_ptr<PanelGroup>& group) \
    { \
        return {}; \
    } \
 \
    template<Derived<Panel> T1, Derived<Panel> T2> \
    std::shared_ptr<PanelGroup> op(std::shared_ptr<T1>& p1, std::shared_ptr<T2>& p2) \
    { \
        return {}; \
    }

PANGO_PANEL_OPERATOR(operator PANGO_COMMA, tabbed)
PANGO_PANEL_OPERATOR(operator|, tabbed)
PANGO_PANEL_OPERATOR(operator/, tabbed)
#undef PANGO_PANEL_OPERATOR
#undef PANGO_COMMA

}