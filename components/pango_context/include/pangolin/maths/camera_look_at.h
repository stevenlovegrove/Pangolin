#pragma once

#include <pangolin/maths/conventions.h>

#include <sophus/lie/se3.h>

// TODO: Will be refactored into Pangolin at some point

namespace pangolin {

// Return a pose (world from camera transform) which
// represents a camera centered at `camera_center_in_world`
// looking at `interest_point_in_world` and whose 'up' direction
// lies in a plane defined by the forward directoin and provided
// `up` vector.
// Special cases:
// * Camera_center_in_world and interest_point_in_world are equal
// * Forward direction and `up` are co-linear
// In these cases, method respects conditions that are possible
// and chooses arbitrarily from underspecified dimensions.
template <typename TScalar=double>
sophus::Se3<TScalar> worldLookatFromCamera(
    Eigen::Vector3<TScalar> const& camera_center_in_world,
    Eigen::Vector3<TScalar> const& interest_point_in_world,
    Eigen::Vector3<TScalar> const& updir_world,
    const DeviceXyz xyz_convention = Conventions::global().device_xyz) {
  using Vec3 = Eigen::Vector3<TScalar>;
  const Vec3 forward_world = interest_point_in_world - camera_center_in_world;
  const Vec3 right_world = forward_world.cross(updir_world);

  if (right_world.squaredNorm() < sophus::kEpsilon<TScalar>) {
    if (forward_world.squaredNorm() < sophus::kEpsilon<TScalar>) {
      // camera and interest point are coincident
      // TODO: choose any orientation
      PANGO_FATAL();
    } else {
      // forward and updir are colinear but distinct
      // TODO: choose any orientation about the forward vector
      PANGO_FATAL();
    }
  }

  // down_world is not necessarily colinear with updir_world, but
  // is the closest vector to updir_world which satisfies the forward
  // direction.
  const Vec3 down_world = forward_world.cross(right_world);

  // This matrix takes us to canonical right,down,forward coordinates
  Eigen::Matrix<TScalar, 3, 3> world_Rmat_rdf;
  world_Rmat_rdf.col(0) = right_world.normalized();
  world_Rmat_rdf.col(1) = down_world.normalized();
  world_Rmat_rdf.col(2) = forward_world.normalized();

  // Convert to requested convention
  Eigen::Matrix<TScalar, 3, 3> world_Rmat_cam;
  if (xyz_convention == DeviceXyz::right_down_forward) {
    world_Rmat_cam = world_Rmat_rdf;
  } else {
    world_Rmat_cam =
        world_Rmat_rdf * toConventionFromRdf<TScalar>(xyz_convention).inverse();
  }

  // Compose rotation and translation
  return sophus::Se3<TScalar>(
      sophus::So3<TScalar>::fitToSo3(world_Rmat_cam), camera_center_in_world);
}

// Convenience version of above, using AxisDirection enum to specify Up vector
template <typename TScalar=double>
sophus::Se3<TScalar> worldLookatFromCamera(
    Eigen::Vector3<TScalar> const& camera_center_in_world,
    Eigen::Vector3<TScalar> const& interest_point_in_world,
    const AxisDirection2 updir_world = Conventions::global().up_direction_world,
    const DeviceXyz xyz_convention = Conventions::global().device_xyz) {
  return worldLookatFromCamera<TScalar>(
      camera_center_in_world,
      interest_point_in_world,
      axisDirection<TScalar,3>(updir_world),
      xyz_convention);
}

// Convenience version of above, using AxisDirection enum to specify Up vector
template <typename TScalar=double>
sophus::Se3<TScalar> cameraLookatFromWorld(
    Eigen::Vector3<TScalar> const& camera_center_in_world,
    Eigen::Vector3<TScalar> const& interest_point_in_world,
    const AxisDirection2 updir_world = Conventions::global().up_direction_world,
    const DeviceXyz xyz_convention = Conventions::global().device_xyz) {
  return worldLookatFromCamera<TScalar>(
      camera_center_in_world,
      interest_point_in_world,
      axisDirection<TScalar,3>(updir_world),
      xyz_convention).inverse();
}

}  // namespace pangolin
