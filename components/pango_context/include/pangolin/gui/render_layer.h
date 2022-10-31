#pragma once

#include <pangolin/utils/shared.h>
#include <pangolin/maths/min_max.h>
#include <Eigen/Core>

namespace pangolin
{

struct Pixels {int pixels = 100;};
struct Parts {double ratio = 1.0; };

////////////////////////////////////////////////////////////////////
/// Represents a client area in a window with layout handling
///
struct RenderLayer : std::enable_shared_from_this<RenderLayer>
{
    virtual ~RenderLayer() {}

    struct RenderParams{
        MinMax<Eigen::Vector2i> region;
    };

    using Dim = std::variant<Parts,Pixels>;
    using Size = Eigen::Vector<Dim,2>;

    virtual void renderIntoRegion(const RenderParams&) = 0;

    virtual Size sizeHint() const = 0;

    struct Params {
        std::string title = "";
        Size size_hint = {Parts{1}, Parts{1}};
    };
    static Shared<RenderLayer> Create(Params);
};

}
