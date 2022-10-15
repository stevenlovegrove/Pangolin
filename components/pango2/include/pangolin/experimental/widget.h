#pragma once

#include "common.h"
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

    static std::shared_ptr<Seperator> Create(Params p);
};

struct Slider : public Widget
{
    struct Params {
        std::string label = "";
        double min=0.0;
        double max=1.0;
    };

    static std::shared_ptr<Slider> Create(Params p);
};

template<Derived<WidgetPanel> T>
void operator+=(const std::shared_ptr<T>& g1, const shared_vector<Widget>& g2)
{
}

}