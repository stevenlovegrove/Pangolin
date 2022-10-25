#pragma once

#include <pangolin/utils/shared.h>
#include <Eigen/Geometry>

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
    struct Absolute {int pixels = 100;};
    struct Parts {double ratio = 1.0; };

    using Dim = std::variant<Parts,Absolute>;
    using Size = Eigen::Vector<Dim,2>;

    struct Params {
        std::string title = "";
        Size size_hint = {Parts{1}, Parts{1}};
    };

    static Shared<Panel> Create(Params p);
};

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct MultiPanel : public Panel
{
    struct Params {
        Panel::Params panel = {};
    };

    static Shared<MultiPanel> Create(Params p);
};

////////////////////////////////////////////////////////////////////
/// Supports displaying a 2D twear-var style interface
///
struct WidgetPanel : public Panel
{
    struct Params {
        Panel::Params panel = {};
        std::string var_subscription;
    };

    static Shared<WidgetPanel> Create(Params p);

    virtual void add(SharedVector<Widget> widgets) = 0;
};

}
