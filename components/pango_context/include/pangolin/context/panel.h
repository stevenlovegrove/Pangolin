#pragma once

#include "common.h"
#include "context.h"

namespace pangolin
{

// Forward declarations
struct Widget;
struct PanelGroup;

////////////////////////////////////////////////////////////////////
/// Represents a client area in a window with layout handling
///
struct Panel : std::enable_shared_from_this<Panel>
{
    struct Params {
        std::string title = "";
    };

    static ExpectShared<Panel> Create(Params p);
};

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct MultiPanel : public Panel
{
    struct Params {
        std::string title = "";
    };

    static ExpectShared<MultiPanel> Create(Params p);
};

////////////////////////////////////////////////////////////////////
/// Supports displaying a 2D twear-var style interface
///
struct WidgetPanel : public Panel
{
    struct Params {
        std::string title = "";
        std::string var_subscription;
    };

    static ExpectShared<WidgetPanel> Create(Params p);

    virtual void add(SharedVector<Widget> widgets) = 0;
};

}