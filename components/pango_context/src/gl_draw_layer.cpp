#include "camera_utils.h"
#include "gl_utils.h"

#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/gl/gl.h>
#include <pangolin/gui/draw_layer.h>
#include <pangolin/gui/draw_layer_handler.h>
#include <pangolin/gui/drawn_image.h>
#include <pangolin/render/device_texture.h>
#include <pangolin/render/projection_lut.h>
#include <pangolin/var/var.h>

#include <unordered_map>

using namespace sophus;

namespace pangolin
{

struct DrawLayerImpl : public DrawLayer {
  DrawLayerImpl(const DrawLayerImpl::Params& p) :
      name_(p.name),
      size_hint_(p.size_hint),
      handler_(p.handler),
      scene_collection_({.drawables = p.in_scene}),
      pixels_collection_({.drawables = p.in_pixels})
  {
    render_state_.aspect_policy = p.aspect_policy;
    render_state_.image_convention = p.image_convention;
    render_state_.image_indexing = p.image_indexing;

    render_state_.near_far = p.near_far;

    if (p.camera) {
      render_state_.camera = *p.camera;
    }

    if (p.camera_from_world) {
      render_state_.camera_from_world = *p.camera_from_world;
    } else {
      render_state_.camera_from_world =
          cameraLookatFromWorld({0.0, 0.0, -5.0}, {0.0, 0.0, 0.0});
    }
  }

  std::string name() const override { return name_; }

  const DrawLayerRenderState& renderState() const override
  {
    return render_state_;
  }

  void setCamera(const sophus::CameraModel& camera) override
  {
    render_state_.camera = camera;
  }

  void setCameraFromWorld(const sophus::Se3F64& cam_from_world) override
  {
    render_state_.camera_from_world = cam_from_world;
  }

  void setClipViewTransform(sophus::Sim2<double>& clip_view_transform) override
  {
    render_state_.clip_view_transform = clip_view_transform;
  }

  void setNearFarPlanes(const RegionF64& near_far) override
  {
    render_state_.near_far = near_far;
  }

  // // Shrinks viewport such that aspect matches the provided camera
  // dimensions.
  // // Will leave padding on in dimension unrendered.
  // static Region2I
  // viewportFromCameraAspect(double cam_aspect, Region2I
  // region)
  // {
  //     const Eigen::Array2i region_size = region.range();
  //     const double region_aspect = double(region_size.x()) /
  //     double(region_size.y()); if(region_aspect > cam_aspect) {
  //         // keep height, change width
  //         const int new_width = int(cam_aspect * region_size.y());
  //         const int new_left = region.min().x() + int((region_size.x() -
  //         new_width) / 2.0); return {
  //             {new_left, region.min().y()},
  //             {new_left+new_width, region.max().y()}
  //         };
  //     }else{
  //         // keep width, change height
  //         const int new_height = int(region_size.x() / cam_aspect);
  //         const int new_bottom = region.min().y() + int((region_size.y() -
  //         new_height) / 2.0); return {
  //             {region.min().x(), new_bottom},
  //             {region.max().x(), new_bottom+new_height }
  //         };
  //     }
  // }

  std::optional<Eigen::Array2i> tryGetDrawableBaseImageSize() const
  {
    Region3F64 pixel_bounds = Region3F64::empty();
    for (const auto& obj : pixels_collection_.drawables) {
      pixel_bounds.extend(obj->boundsInParent());
    }
    const Eigen::Array2i dim = pixel_bounds.range().head<2>().cast<int>();
    if (0 < dim[0] && 0 < dim[1]) return dim;
    return std::nullopt;
  }

  bool tryInitializeEmptyCamera()
  {
    if (render_state_.camera.isEmpty()) {
      if (auto maybe_dim = tryGetDrawableBaseImageSize()) {
        sophus::ImageSize size = toImageSize(*maybe_dim);
        render_state_.camera = sophus::createDefaultPinholeModel(size);
        return true;
      } else if (scene_collection_.drawables.size()) {
        render_state_.camera = sophus::createDefaultPinholeModel({640, 480});
        return true;
      }
      return false;
    }
    return true;
  }

  struct RenderData {
    Eigen::Array2d clip_aspect_scale = {1.0, 1.0};
    Eigen::Matrix4d clip_view = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d clip_aspect = Eigen::Matrix4d::Identity();
    ViewParams pixel_params;
    ViewParams scene_params;
    Shared<DeviceTexture> unproject_map = DeviceTexture::Create({});
  };

  static Eigen::Matrix4d sim2To4x4(Sim2<double> sim2)
  {
    const Eigen::Matrix3d m3 = sim2.matrix();
    Eigen::Matrix4d ret = Eigen::Matrix4d::Identity();
    ret.topLeftCorner<2, 2>() = m3.topLeftCorner<2, 2>();
    ret.topRightCorner<2, 1>() = m3.topRightCorner<2, 1>();
    return ret;
  }

  static void updateRenderData(
      RenderData& state, const DrawLayerRenderState& render_state,
      Region2I viewport)
  {
    state.clip_view = sim2To4x4(render_state.clip_view_transform);
    state.clip_aspect_scale =
        axisScale(viewport.range(), toEigen(render_state.camera.imageSize()));

    switch (render_state.aspect_policy) {
      case AspectPolicy::stretch:
        // Nothing to do
        break;
      case AspectPolicy::crop:
        PANGO_UNIMPLEMENTED();
        break;
      case AspectPolicy::overdraw:
        // We'll modify clip coords to fix aspect (extending field of view)
        state.clip_aspect = Eigen::Vector4d(
                                state.clip_aspect_scale.x(),
                                state.clip_aspect_scale.y(), 1.0, 1.0)
                                .asDiagonal();
        break;
    }

    if (state.unproject_map->empty() &&
        render_state.camera.distortionType() != CameraDistortionType::pinhole) {
      std::visit(
          [&](const auto& camera) {
            auto unprojmap = Image<Pixel3<float>>::makeGenerative(
                render_state.camera.imageSize(), [&](int x, int y) {
                  auto p_img = Eigen::Vector2d(x, y);
                  auto p_cam = camera.camUnproj(p_img, 1.0);
                  return p_cam.template cast<float>().eval();
                });
            state.unproject_map->update(unprojmap);
          },
          render_state.camera.modelVariant());
      state.unproject_map->sync();
      PANGO_GL(
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      PANGO_GL(
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    }

    const GraphicsProjection proj_type =
        render_state.camera.distortionType() ==
                sophus::CameraDistortionType::orthographic
            ? GraphicsProjection::orthographic
            : GraphicsProjection::perspective;

    const auto clip_from_projection = transformClipFromProjection(
        render_state.camera.imageSize(), render_state.image_convention,
        render_state.image_indexing);

    state.pixel_params = {
        .viewport = viewport,
        .camera_dim = render_state.camera.imageSize(),
        .near_far = {-2.0, 2.0},
        .camera_from_drawable = Eigen::Matrix4d::Identity(),
        .image_from_camera = Eigen::Matrix4d::Identity(),
        .clip_from_image = state.clip_view * state.clip_aspect *
                           clip_from_projection *
                           transformProjectionFromImage(
                               {-2.0, 2.0}, GraphicsProjection::orthographic)};

    state.scene_params = {
        .viewport = viewport,
        .camera_dim = render_state.camera.imageSize(),
        .near_far = render_state.near_far,
        .camera_from_drawable = render_state.camera_from_world.matrix(),
        .image_from_camera = transformImageFromCamera4x4(render_state.camera),
        .clip_from_image =
            state.clip_view * state.clip_aspect * clip_from_projection *
            transformProjectionFromImage(render_state.near_far, proj_type)};

    if (!state.unproject_map->empty()) {
      state.scene_params.unproject_map = state.unproject_map;
    }
  }

  void renderIntoRegion(
      const Context& context, const RenderParams& params) override
  {
    if (!tryInitializeEmptyCamera()) {
      return;
    }

    updateRenderData(render_data_, render_state_, params.region);

    ScopedGlEnable en_scissor(GL_SCISSOR_TEST);

    ////////////////////////////////////////////////////////////////////////
    if (pixels_collection_.drawables.size()) {
      context.setViewport(render_data_.pixel_params.viewport);
      auto child_params = render_data_.pixel_params;
      for (auto& obj : pixels_collection_.drawables) {
        child_params.camera_from_drawable =
            render_data_.pixel_params.camera_from_drawable *
            obj->pose.parentFromDrawableMatrix();
        obj->draw(child_params);
      }

      if (scene_collection_.drawables.size()) {
        // We'll clear depth buffer for use for in_scene_ Drawables
        glClear(GL_DEPTH_BUFFER_BIT);
      }
    }

    if (scene_collection_.drawables.size()) {
      context.setViewport(render_data_.scene_params.viewport);
      auto child_params = render_data_.scene_params;
      for (auto& obj : scene_collection_.drawables) {
        child_params.camera_from_drawable =
            render_data_.scene_params.camera_from_drawable *
            obj->pose.parentFromDrawableMatrix();
        obj->draw(child_params);
      }
    }
  }

  bool handleEvent(const Context& context, const Event& event) override
  {
    Eigen::Matrix4d clip_from_proj =
        render_data_.clip_view * render_data_.clip_aspect *
        transformClipFromProjection(render_state_.camera.imageSize());
    const Eigen::Array2d d =
        event.pointer_pos.pos_window.cast<double>().array() -
        sophus::cast<double>(event.pointer_pos.region.min()).array();
    const Eigen::Array2d pos_zero_one =
        d / sophus::cast<double>(event.pointer_pos.region.range()).array();

    const Eigen::Array2d pos_clip =
        (pos_zero_one * 2.0 - Eigen::Array2d(1.0, 1.0)) *
        Eigen::Array2d(1.0, -1.0);
    const Eigen::Array2d pos_img =
        (clip_from_proj.inverse() *
         Eigen::Vector4d(pos_clip.x(), pos_clip.y(), -1.0, 1.0))
            .head<2>();

    if (handler_->viewMode() == ViewMode::best_guess) {
      handler_->setViewMode(
          pixels_collection_.drawables.size() ? ViewMode::image_plane
                                              : ViewMode::freeview);
    }

    Eigen::Matrix3d clip_from_window, pixel_from_window;
    return handler_->handleEvent(
        context, event, pos_clip, pos_img, render_data_.clip_aspect_scale,
        *this, render_state_);
  }

  Size sizeHint() const override { return size_hint_; }

  double aspectHint() const override
  {
    Region3F64 cam_bounds = Region3F64::empty();
    for (const auto& obj : pixels_collection_.drawables) {
      cam_bounds.extend(obj->boundsInParent());
    }
    if (!cam_bounds.isEmpty()) {
      Eigen::Vector2d dim = cam_bounds.range().head<2>();
      return dim.x() / dim.y();
    }
    if (render_state_.camera.imageSize().width &&
        render_state_.camera.imageSize().height) {
      return (double)render_state_.camera.imageSize().width /
             (double)render_state_.camera.imageSize().height;
    }
    return 1.0;
  };

  void add(
      const Shared<Drawable>& r, In domain, const std::string& name) override
  {
    if (!name.empty()) {
      // has a name, remove old Drawables with that name
      remove(name);
    }
    switch (domain) {
      case In::scene: {
        this->scene_collection_.drawables.push_back(r);
        if (!name.empty()) {
          this->scene_collection_.named_drawables.insert({name, r});
        }
      } break;
      case In::pixels: {
        this->pixels_collection_.drawables.push_back(r);
        if (!name.empty()) {
          this->pixels_collection_.named_drawables.insert({name, r});
        }
      } break;
    }
  }

  std::shared_ptr<Drawable> get(const std::string& name) const override
  {
    for (auto& map :
         {scene_collection_.named_drawables,
          pixels_collection_.named_drawables}) {
      auto it = map.find(name);
      if (it != map.end()) {
        return it->second;
      }
    }
    return nullptr;
  }

  bool remove(const Shared<Drawable>& r) override
  {
    bool anything_erased = false;
    anything_erased |= pixels_collection_.remove(r);
    anything_erased |= scene_collection_.remove(r);

    return anything_erased;
  }

  bool remove(const std::string& name) override
  {
    auto maybe_shared = get(name);
    if (maybe_shared == nullptr) {
      return false;
    }
    return remove(maybe_shared);
  }

  void clear(std::optional<In> domain = std::nullopt) override
  {
    if (domain) {
      if (*domain == In::pixels) {
        pixels_collection_.clear();
      } else if (*domain == In::scene) {
        scene_collection_.clear();
      }
    } else {
      pixels_collection_.clear();
      scene_collection_.clear();
    }
  }

  struct DrawableCollection {
    std::vector<Shared<Drawable>> drawables;
    std::map<std::string, Shared<Drawable>> named_drawables;

    void clear()
    {
      drawables.clear();
      named_drawables.clear();
    }

    bool remove(const Shared<Drawable>& r) noexcept
    {
      bool anything_erased = false;

      {
        auto end = std::remove(drawables.begin(), drawables.end(), r);
        anything_erased |= end != drawables.end();
        drawables.erase(end, drawables.end());
      }
      {
        std::string key;
        for (const auto& e : named_drawables) {
          if (e.second == r) {
            key = e.first;
          }
        }
        named_drawables.erase(key);
      }

      return anything_erased;
    }
  };

  void updateBackgroundImage(const sophus::IntensityImage<>& image) override
  {
    const sophus::CameraModel& camera = defaultOrthoCameraForImage(image);
    this->setCamera(camera);
    this->addNamedInPixels("---unique-background-image---", image);
  }

  std::string name_;
  Size size_hint_;

  DrawLayerRenderState render_state_;
  RenderData render_data_;

  Shared<DrawLayerHandler> handler_;

  DrawableCollection scene_collection_;
  DrawableCollection pixels_collection_;
};

PANGO_CREATE(DrawLayer) { return Shared<DrawLayerImpl>::make(p); }

}  // namespace pangolin
