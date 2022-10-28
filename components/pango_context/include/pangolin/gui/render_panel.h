#pragma once

#include <pangolin/maths/min_max.h>
#include <pangolin/gui/panel.h>
#include <pangolin/gui/renderable.h>
#include <sophus/image/image.h>

namespace pangolin
{

struct Handler;

// TODO: This stuff is great for framebuffer rendering etc too.
//       This should be an object independent of the GUI.
//       Lets just have all panels contain a Render object,
//       potentially a nullable one.
//
//       Scratch that - let's just rename Panel to be RenderLayer
//       the layout engine can already deal with stacks
//
//       Then we just need a kind of stack that supports a deferred
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

// TODO: Would a command queue style system be more sensible, where a
//       user specifies what to update?

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct RenderPanel : public Panel
{
    struct Lut {
        // Maps input pixel (u,v) to a direction vector (dx,dy,dz)
        // in camera frame. We use a vector ray for simplicity and
        // speed
        std::optional<sophus::Image<Eigen::Vector3f>> unproject;

        // Maps ray angle (theta,phi) to pixel (u,v).
        // We use an angular map representation here despite its
        // tradeoffs to keep the input space bounded and to support
        // fisheye distortions
        std::optional<sophus::Image<Eigen::Vector2f>> project;

        // Represents a per-pixel scale factor
        std::optional<sophus::Image<float>> vignette;
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

    // Specify a handler to feed user input into this RenderPanel
    // to drive the tranforms and view options.
    virtual void setHandler(const std::shared_ptr<Handler>&) = 0;

    // This method is the main way in which a handler object
    // can constrain viewing parameters to relevant content.
    virtual MinMax<Eigen::Vector3d> getSceneBoundsInWorld() const = 0;

    // // get (and create if it doesn't exit) a renderable container to fill or update.
    // // (group_key,object_key) form a key pair which can be used for removing or updating later.
    // // Elements in a group can be enabled / disabled collectively
    // virtual Renderable& getRenderableVariant(const std::string& group_key, size_t object_key = 0) = 0;

    // // Remove the Renderable from the panel, deallocating any host and graphics memory
    // // that it may have been using.
    // virtual void removeRenderable(const std::string& group_key, size_t object_key = 0) = 0;

    // // Convenience template. Will throw if type is incorrect
    // template<typename TRenderable>
    // TRenderable& getRenderable(const std::string& group_key, size_t object_key = 0)
    // {
    //     return std::get<TRenderable>(getRenderableVariant(group_key, object_key));
    // }

    struct Params {
        Panel::Params panel = {};
    };
    static Shared<RenderPanel> Create(Params p);
};

}
