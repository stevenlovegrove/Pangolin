#pragma once

#include <vector>

#include <sophus/image/runtime_image.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/render/device_buffer.h>
#include <pangolin/render/device_texture.h>
#include <pangolin/render/colormap.h>

#include <pangolin/gui/draw_layer.h>

namespace pangolin
{

struct DrawnSolids : public DrawLayer::Drawable
{
    enum class Type {
        checkerboard
    };

    Type object_type;

    struct Params {
        Type object_type = Type::checkerboard;
        Eigen::Matrix4d parent_from_drawable = Eigen::Matrix4d::Identity();
    };
    static Shared<DrawnSolids> Create(Params p);
};

}
