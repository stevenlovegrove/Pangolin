// Copyright (c) farm-ng, inc. All rights reserved.

#pragma once

#include <pangolin/utils/logging.h>
#include <sophus/common/common.h>
#include <sophus/common/enum.h>
#include <sophus/linalg/orthogonal.h>

namespace pangolin
{

// clang-format off
SOPHUS_ENUM(
  AxisDirection,
  (positive_x,
   positive_y,
   positive_z,
   negative_x,
   negative_y,
   negative_z)
);

// Defines the semantic meaning of the coordinate axis
// in relation to a physical camera or other device
SOPHUS_ENUM(
  DeviceXyz,
  (right_down_forward,        // Common in computer vision, where forward is the principle ray.
   forward_left_up,           // Common in robotics from a robotic egocentric perspective
   right_up_back)             // Used extensively with OpenGL
);


SOPHUS_ENUM(
  Handed,
  (left,
   right)
);

// Describes the semantic meaning of an images coordinate axis
SOPHUS_ENUM(
  ImageXy,
  (right_down,                // Common in computer vision
   right_up)                  // Common in graphics
);

// Describes how image indexing relates to discrete pixels in memory
SOPHUS_ENUM(
  ImageIndexing,
  (normalized_zero_one,       // x and y lie in the range [ 0,1] inclusive
   normalized_minus_one_one,  // x and y lie in the range [-1,1] inclusive
   pixel_centered,            // integral index locations lie at pixel centers
   pixel_continuous)          // integral index locations lie at corners. (0,0)
);                            //      will lie at the corner of an edge pixel

SOPHUS_ENUM(
    DistanceUnits,
    (millimeters,
     centimeters,
     meters,
     kilometers,
     other)
);

SOPHUS_ENUM(
  GraphicsProjection,
  (perspective,
   orthographic)
);

// clang-format on

template <typename TScalar, int Dim>
Eigen::Matrix<TScalar, Dim, 1> axisDirection(AxisDirection dir)
{
  const int ordinal = static_cast<int>(dir);  // [0, ... 2*Dim]
  const int axis = ordinal % 3;               // [0, ... Dim]
  const int pos_neg = ordinal / 3;            // [0, 1]
  const int scale = -2 * pos_neg + 1;         // [-1, +1]
  return scale * Eigen::Matrix<TScalar, Dim, Dim>::Identity().col(axis);
}

template <typename TScalar>
Eigen::Vector3<TScalar> upDirectionInCamera(DeviceXyz axis_convention)
{
  const static Eigen::Vector3<TScalar> up_dirs[3] = {
      {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}};

  const size_t index = static_cast<size_t>(axis_convention);
  PANGO_ENSURE(index < 3);
  return up_dirs[index];
}

// Creates a matrix which transforms vectors defined
// in an RDF (x=right,y=down,z=forward) frame into one
// where those now point in the `to_rdf_axis_dirs` dirs
template <typename TScalar, int Dim = 3>
Eigen::Matrix<TScalar, Dim, Dim> toConventionFromRdf(
    const std::array<AxisDirection, Dim>& to_rdf_axis_dirs)
{
  static_assert(2 <= Dim && Dim <= 3);

  Eigen::Matrix<TScalar, Dim, Dim> to_Rmat_rdf;
  for (int i = 0; i < Dim; ++i) {
    to_Rmat_rdf.row(i) = axisDirection<TScalar, Dim>(to_rdf_axis_dirs[i]);
  }
  return to_Rmat_rdf;
}

template <typename TScalar>
Eigen::Matrix<TScalar, 3, 3> toConventionFromRdf(DeviceXyz to)
{
  switch (to) {
    case DeviceXyz::right_down_forward:
      return toConventionFromRdf<TScalar, 3>(
          {AxisDirection::positive_x, AxisDirection::positive_y,
           AxisDirection::positive_z});
    case DeviceXyz::forward_left_up:
      return toConventionFromRdf<TScalar, 3>(
          {AxisDirection::positive_z, AxisDirection::negative_x,
           AxisDirection::negative_y});
    case DeviceXyz::right_up_back:
      return toConventionFromRdf<TScalar, 3>(
          {AxisDirection::positive_x, AxisDirection::negative_y,
           AxisDirection::negative_z});
    default:
      PANGO_FATAL();
  }
}

template <typename TScalar>
Eigen::Matrix<TScalar, 3, 3> toConventionFrom(DeviceXyz to, DeviceXyz from)
{
  return toConventionFromRdf<TScalar>(to) *
         toConventionFromRdf<TScalar>(from).inverse();
}

struct Conventions {
  // Global singleton to provide defaults elsewhere
  static Conventions& global();

  // Default values at program startup
  DeviceXyz device_xyz = DeviceXyz::right_down_forward;
  ImageXy image_xy = ImageXy::right_down;
  ImageIndexing image_indexing = ImageIndexing::pixel_centered;
  DistanceUnits units = DistanceUnits::meters;
  AxisDirection up_direction_world = AxisDirection::positive_z;
};

}  // namespace pangolin
