#pragma once

#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/min_max.h>
#include <pangolin/gui/layer_group.h>
#include <pangolin/gui/draw_layer_handler.h>
#include <pangolin/gui/drawable.h>
#include <sophus/sensor/camera_model.h>
#include <sophus/lie/se3.h>
#include <sophus/lie/sim2.h>

#include <variant>

namespace pangolin
{

template<typename T>
struct DrawableConversionTraits;

struct DeviceTexture;

struct DrawLayerRenderState {
    sophus::CameraModel camera;
    sophus::SE3d camera_from_world;
    sophus::Sim2<double> clip_view_transform;
    MinMax<double> near_far;
};

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct DrawLayer : public Layer
{
    enum In {
        scene,
        pixels
    };

    enum AspectPolicy {
        stretch,
        crop,
        overdraw,
        mask
    };

    virtual const DrawLayerRenderState& renderState() const = 0;
    virtual void setCamera(const sophus::CameraModel&) = 0;
    virtual void setCameraFromWorld(const sophus::Se3<double>&) = 0;
    virtual void setClipViewTransform(sophus::Sim2<double>&) = 0;
    virtual void setNearFarPlanes(const MinMax<double>&) = 0;

    virtual void add(const Shared<Drawable>& r, In domain, const std::string& name = "") = 0;
    virtual std::shared_ptr<Drawable> get(const std::string& name) const = 0;
    virtual bool remove(const Shared<Drawable>& r) = 0;
    virtual void clear(std::optional<In> domain = std::nullopt) = 0;

    // convenience method to remove by name
    inline bool remove(const std::string& name) {
        auto maybe_shared  = get(name);
        if (maybe_shared == nullptr) {
            return false;
        }
        return remove(maybe_shared);
    }

    // Convenience method to add several drawables together
    template<typename ...Ts>
    void addInScene(const Ts&... ts) {
        (add(DrawableConversionTraits<Ts>::makeDrawable(ts), In::scene), ...);
    }

    template<typename T>
    void addNamedInScene(const std::string& name, const T& r) {
        add(DrawableConversionTraits<T>::makeDrawable(r), In::scene, name);
    }

    template<typename T>
    void addNamedInPixels(const std::string& name, const T& r) {
        add(DrawableConversionTraits<T>::makeDrawable(r), In::pixels, name);
    }


    // Convenience method to add several drawables together
    template<typename ...Ts>
    void addInPixels(const Ts&... ts) {
        (add(DrawableConversionTraits<Ts>::makeDrawable(ts), In::pixels), ...);
    }

    struct Params {
        std::string name = "";
        Size size_hint = {Parts{1}, Parts{1}};
        AspectPolicy aspect_policy = AspectPolicy::mask;
        Shared<DrawLayerHandler> handler = DrawLayerHandler::Create({});

        std::optional<sophus::CameraModel> camera = std::nullopt;
        std::optional<sophus::Se3F64> camera_from_world = std::nullopt;
        MinMax<double> near_far = {1e-3, 1e6};

        // Objects to draw through modelview transform
        std::vector<Shared<Drawable>> in_scene = {};

        // Objects to draw in camera-frame, including 2D image-plane drawing.
        // These are 'camera pixels', not 'screen pixels'.
        std::vector<Shared<Drawable>> in_pixels = {};
    };
    static Shared<DrawLayer> Create(Params p);
};

////////////////////////////////////////////////////////////////////////
// Traits for extending what types can be converted to drawables

template<DerivedFrom<Drawable> L>
struct DrawableConversionTraits<Shared<L>> {
static Shared<Drawable> makeDrawable(const Shared<L>& x) {
    return x;
}};

template<DerivedFrom<Drawable> L>
struct DrawableConversionTraits<std::shared_ptr<L>> {
static Shared<Drawable> makeDrawable(const std::shared_ptr<L>& x) {
    return x;
}};

template<typename T>
concept DrawableConvertable = requires (T x) {
    {DrawableConversionTraits<T>::makeDrawable(x)} -> SameAs<Shared<Drawable>>;
};

template<DrawableConvertable T>
Shared<Drawable> makeDrawable(const T& v) {
    return DrawableConversionTraits<T>::makeDrawable(v);
}

////////////////////////////////////////////////////////////////////////
// Direct conversion of anything drawable to a layer

// Helper for adding Drawables directly into layouts
template<DerivedFrom<Drawable> T>
struct LayerConversionTraits<Shared<T>> {
    static Shared<Layer> makeLayer(const Shared<T>& drawable) {
        return DrawLayer::Create({ .in_scene = {makeDrawable(drawable)} });
    }
};

}
