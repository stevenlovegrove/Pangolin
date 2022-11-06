// Copyright (c) farm-ng, inc. All rights reserved.

#include <pangolin/maths/camera_look_at.h>
#include <pangolin/testing/eigen.h>

#include "test_data.h"

using namespace Eigen;
using namespace sophus;

namespace pangolin {

template <typename T>
void testForParams(
    Vector3<T> const& lookat_in_world,
    Vector3<T> const& camera_in_world,
    Vector3<T> const& up_in_world,
    DeviceXyz convention) {
  Se3<T> const world_pose_cam = worldLookatFromCamera(
      camera_in_world, lookat_in_world, up_in_world, convention);
  Se3<T> const cam_pose_world = world_pose_cam.inverse();
  Vector3<T> const lookat_in_cam = cam_pose_world * lookat_in_world;

  // defines (in the rows) the directions we expect for right, down and forward
  // in camera frame
  const Eigen::Matrix3<T> conv_R_rdf = toConventionFromRdf<T>(convention);

  // lookat point should be colinear with forward vector in camera frame
  const Eigen::Vector3<T> fwd_conv = conv_R_rdf.row(2);
  const T look_dot_fwd = fwd_conv.dot(lookat_in_cam.normalized());
  CHECK(look_dot_fwd > 0.0);
  CHECK(T(1.0) - std::abs(look_dot_fwd) < sophus::kEpsilon<T>);

  // check right vector is orthogonal forward
  const Eigen::Vector3<T> right_conv = conv_R_rdf.row(0);
  CHECK(std::abs(fwd_conv.dot(right_conv)) < sophus::kEpsilon<T>);
}

void simpleTest() {
  Vector3<double> const lookat_in_world(0.0, 0.0, 1.0);
  Vector3<double> const camera_in_world(0.0, 0.0, 0.0);
  Vector3<double> const up_in_world(0.0, -1.0, 0.0);
  const Se3F64 world_pose_cam = worldLookatFromCamera(
      camera_in_world,
      lookat_in_world,
      up_in_world,
      DeviceXyz::right_down_forward);

  CHECK_EIGEN_APPROX(world_pose_cam.matrix(), Eigen::Matrix4d::Identity());
}

template <typename T>
void testForEachConvention(DeviceXyz Convention) {
  auto test_vec3 = testMats<T, 3, 1>(testVecs(), 100);

  for (size_t i = 0; i < test_vec3.size() - 3; ++i) {
    testForParams(
        // Vector3<T>(5.0, 4.0, 1.0),
        // Vector3<T>(1.0,2.0,3.0),
        // Vector3<T>(0.0, 1.0, 0.0),
        test_vec3[i + 0],
        test_vec3[i + 1],
        test_vec3[i + 2],
        DeviceXyz::right_down_forward);
  }
}

template <typename T>
void testForScalar() {
  for (DeviceXyz c : getAll(DeviceXyz())) {
    testForEachConvention<T>(c);
  }
}

TEST_CASE("lookat, forward") {
  simpleTest();

  // testForScalar<float>();
  testForScalar<double>();
}

}  // namespace pangolin
