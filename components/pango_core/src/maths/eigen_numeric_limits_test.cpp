// Copyright (c) farm-ng, inc. All rights reserved.

#include <pangolin/testing/eigen.h>
#include <pangolin/maths/eigen_numeric_limits.h>
#include <pangolin/maths/eigen_scalar_methods.h>

namespace pangolin {

template <typename T>
struct CommonTestsForType {
  static void run() {
    T const lowest = MultiDimLimits<T>::lowest();
    T const min = MultiDimLimits<T>::min();
    T const max = MultiDimLimits<T>::max();

    CHECK(reduce(
        min, max, true, [](auto a, auto b, bool& r) { r = r && (a < b); }));
    CHECK_FALSE(reduce(
        min, max, true, [](auto a, auto b, bool& r) { r = r && (a > b); }));
    CHECK(reduce(
        lowest, min, true, [](auto a, auto b, bool& r) { r = r && (a <= b); }));
    CHECK(reduce(
        lowest, max, true, [](auto a, auto b, bool& r) { r = r && (a < b); }));
  }
};

TEST_CASE("Limits") {
  CommonTestsForType<double>::run();
  CommonTestsForType<float>::run();
  CommonTestsForType<int32_t>::run();
  CommonTestsForType<int16_t>::run();
  CommonTestsForType<uint32_t>::run();
  CommonTestsForType<uint16_t>::run();
  CommonTestsForType<Eigen::Vector2d>::run();
  CommonTestsForType<Eigen::Vector3d>::run();
  CommonTestsForType<Eigen::Vector4d>::run();
  CommonTestsForType<Eigen::Matrix2d>::run();
  CommonTestsForType<Eigen::Matrix3d>::run();
  CommonTestsForType<Eigen::Matrix4d>::run();
  CommonTestsForType<Eigen::Vector2f>::run();
  CommonTestsForType<Eigen::Vector3f>::run();
  CommonTestsForType<Eigen::Vector4f>::run();
  CommonTestsForType<Eigen::Matrix2f>::run();
  CommonTestsForType<Eigen::Matrix3f>::run();
  CommonTestsForType<Eigen::Matrix4f>::run();

  CommonTestsForType<Eigen::Matrix<int32_t, 3, 4>>::run();
  CommonTestsForType<Eigen::Matrix<uint16_t, 6, 2>>::run();
}

}  // namespace pangolin
