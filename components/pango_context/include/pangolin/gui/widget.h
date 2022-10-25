#pragma once

#include "panel.h"

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Represents a simple user interface element
///
struct Widget : std::enable_shared_from_this<Widget>
{
};

struct Seperator : public Widget
{
    struct Params {
        std::string label = "";
    };

    static Shared<Seperator> Create(Params p);
};

struct Slider : public Widget
{
    struct Params {
        std::string label = "";
        double min=0.0;
        double max=1.0;
    };

    static Shared<Slider> Create(Params p);
};

template<std::derived_from<WidgetPanel> T>
void operator+=(const Shared<T>& g1, const SharedVector<Widget>& g2)
{
}

}
