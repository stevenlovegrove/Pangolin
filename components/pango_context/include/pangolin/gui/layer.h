#pragma once

#include <pangolin/utils/shared.h>
#include <pangolin/maths/min_max.h>
#include <Eigen/Core>

#include <variant>


namespace pangolin
{

struct Pixels {int pixels = 100;};
struct Parts {double ratio = 1.0; };

////////////////////////////////////////////////////////////////////
/// Represents a client area in a window with layout handling
///
struct Layer
{
    virtual ~Layer() {}

    using Dim = std::variant<Parts,Pixels>;
    using Size = Eigen::Vector<Dim,2>;

    struct RenderParams{
        MinMax<Eigen::Vector2i> region;
    };
    virtual void renderIntoRegion(const RenderParams&) = 0;

    virtual Size sizeHint() const = 0;

    struct Params {
        std::string title = "";
        Size size_hint = {Parts{1}, Parts{1}};
    };
    static Shared<Layer> Create(Params);
};

}
