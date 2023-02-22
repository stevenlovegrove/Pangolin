#pragma once

#include <pangolin/render/device_buffer.h>

#include <variant>

namespace pangolin
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

using NonLinearMethod = std::variant<std::monostate, Lut>;

}  // namespace pangolin
