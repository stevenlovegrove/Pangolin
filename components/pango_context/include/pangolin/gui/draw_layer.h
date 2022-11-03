#pragma once

#include <pangolin/maths/min_max.h>
#include <pangolin/gui/layer.h>
#include <pangolin/gui/drawable.h>
#include <pangolin/handler/handler.h>
#include <pangolin/handler/interactive.h>

#include <sophus/image/image.h>

#include <variant>

namespace pangolin
{

struct Handler;

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct DrawLayer : public Layer, Interactive
{
    struct Lut {
        // Maps input pixel (u,v) to a direction vector (dx,dy,dz)
        // in camera frame. We use a vector ray for simplicity and
        // speed
        Shared<DeviceBuffer> unproject;

        // Maps ray angle (theta,phi) to pixel (u,v).
        // We use an angular map representation here despite its
        // tradeoffs to keep the input space bounded and to support
        // fisheye distortions
        Shared<DeviceBuffer> project;

        // Represents a per-pixel scale factor
        Shared<DeviceBuffer> vignette;
    };
    using NonLinearMethod = std::variant<std::monostate,Lut>;

    // Specify how objects are projected into the virtual camera,
    // or ray-traced from it (in which case intrinsic_k will be
    // inverted).
    virtual void setProjection(
        const Eigen::Matrix4d& intrinsic_k,
        const NonLinearMethod non_linear = {},
        double duration_seconds = 0.0) = 0;

    // Specify the tranform which takes scene objects from
    // a common 'world' frame into the viewing camera frame.
    virtual void setCamFromWorld(
        const Eigen::Matrix4d& cam_from_world,
        double duration_seconds = 0.0) = 0;

    // Specify a handler to feed user input into this DrawLayer
    // to drive the tranforms and view options.
    virtual void setHandler(const std::shared_ptr<Handler>&) = 0;

    // This method is the main way in which a handler object
    // can constrain viewing parameters to relevant content.
    virtual MinMax<Eigen::Vector3d> getSceneBoundsInWorld() const = 0;

    virtual void add(const Shared<Drawable>& r) = 0;

    virtual void remove(const Shared<Drawable>& r) = 0;

    virtual void clear() = 0;

    struct Params {
        std::string title = "";
        Size size_hint = {Parts{1}, Parts{1}};
        std::shared_ptr<Handler> handler = Handler::Create({});
        Eigen::Matrix4d cam_from_world = Eigen::Matrix4d::Identity();
        Eigen::Matrix4d intrinsic_k = Eigen::Matrix4d::Identity();
        NonLinearMethod non_linear = {};
        std::vector<Shared<Drawable>> objects = {};
    };
    static Shared<DrawLayer> Create(Params p);
};

}
