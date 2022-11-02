#pragma once

#include <pangolin/maths/min_max.h>
#include <pangolin/gui/render_layer.h>
#include <pangolin/gui/renderable.h>
#include <sophus/image/image.h>

#include <variant>

namespace pangolin
{

struct Handler;

// TODO: We just need a kind of stack that supports a deferred
//       render stage, with pipelining? Need a renderable which
//       can take a gpu resource directly.

// TODO: Have a texture cache which lets you pass images into methods
//       that would take textures - it uploads if needed, or uses whats
//       in the cache. A weak ptr lets the texture get deallocated when
//       the corresponding host image is deleted. Would need a way to
//       get around it being too smart...

// TODO: How is the implementation meant to know when a user
//       updates a renderable and it needs to get re-uploaded?
//       How can a user partially update something? If everthing was
//       nullable once uplaoded, the implementation could check if
//       things get set.

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct DrawLayer : public RenderLayer
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

    virtual void add(const Shared<Renderable>& r) = 0;

    virtual void remove(const Shared<Renderable>& r) = 0;

    virtual void clear() = 0;

    struct Params {
        std::string title = "";
        Size size_hint = {Parts{1}, Parts{1}};
        std::shared_ptr<Handler> handler = nullptr;
        Eigen::Matrix4d cam_from_world = Eigen::Matrix4d::Identity();
        Eigen::Matrix4d intrinsic_k = Eigen::Matrix4d::Identity();
        NonLinearMethod non_linear = {};
        std::vector<Shared<Renderable>> objects = {};
    };
    static Shared<DrawLayer> Create(Params p);
};

}