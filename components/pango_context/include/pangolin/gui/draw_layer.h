#pragma once

#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/min_max.h>
#include <pangolin/gui/layer.h>
#include <pangolin/gui/drawable.h>
#include <pangolin/handler/handler.h>
#include <sophus/sensor/camera_model.h>
#include <sophus/lie/se3.h>

#include <variant>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct DrawLayer : public Layer
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

    // Specify a handler to feed user input into this DrawLayer
    // to drive the tranforms and view options.
    virtual void setHandler(const std::shared_ptr<Handler>&) = 0;
    virtual std::shared_ptr<Handler> getHandler() const = 0;

    // Set the camera to use for rendering
    virtual void setCamera(std::shared_ptr<sophus::CameraModel>&) = 0;
    virtual std::shared_ptr<sophus::CameraModel> getCamera() const = 0;

    // Set the view transform to use for rendering
    virtual void setCameraPoseFromWorld( std::shared_ptr<sophus::Se3F64>&) = 0;
    virtual std::shared_ptr<sophus::Se3F64> getCameraPoseFromWorld() const = 0;

    // This method is the main way in which a handler object
    // can constrain viewing parameters to relevant content.
    virtual MinMax<Eigen::Vector3d> getSceneBoundsInWorld() const = 0;

    virtual void add(const Shared<Drawable>& r) = 0;

    virtual void remove(const Shared<Drawable>& r) = 0;

    virtual void clear() = 0;

    struct Params {
        std::string name = "";
        Size size_hint = {Parts{1}, Parts{1}};

        // Leave these all as default for smart initialization
        MinMax<double> near_far = {};
        std::shared_ptr<sophus::CameraModel> camera = nullptr;
        std::shared_ptr<sophus::Se3F64> camera_from_world = nullptr;
        MinMax<Eigen::Vector3d> camera_limits_in_world = {};
        std::shared_ptr<Handler> handler = Handler::Create({});

        // Objects to draw
        std::vector<Shared<Drawable>> objects = {};
    };
    static Shared<DrawLayer> Create(Params p);
};

}
