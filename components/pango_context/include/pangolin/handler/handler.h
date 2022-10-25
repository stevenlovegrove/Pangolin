// Copyright (c) farm-ng, inc. All rights reserved.

#pragma once

#include <pangolin/render/framebuffer_reader.h>
#include <pangolin/handler/handler.h>
#include <sophus/sensor/camera_model.h>

// TODO: Will be refactored into Pangolin at some point
namespace pangolin {

class Handler {
 public:
  // Equivelent to ModelView matrix
  virtual sophus::Se3F64 const &worldFromCamera() const = 0;

  virtual FramebufferReader const &framebufferReader() const = 0;

  virtual sophus::CameraModel const &camera() const = 0;

  struct Params {
    std::shared_ptr<FramebufferReader> framebuffer_reader;
    std::shared_ptr<sophus::CameraModel> camera;
    std::shared_ptr<sophus::Se3F64> world_from_camera;
  };
  static std::unique_ptr<Handler> Create(Params const &);
};

}  // namespace pangolin
