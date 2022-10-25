// Copyright (c) farm-ng, inc. All rights reserved.

#include "coord_convention.h"

#include "test_data.h"

#include <gtest/gtest.h>

using namespace Eigen;
using namespace sophus;

namespace pangolin {

template <typename T>
void testAxisDirection() {
  EXPECT_MAT_EQ(
      (axisDirection<T, 3>(AxisDirection2::positive_x)),
      (Eigen::Vector<T, 3>(1.0, 0.0, 0.0)));

  EXPECT_MAT_EQ(
      (axisDirection<T, 3>(AxisDirection2::positive_y)),
      (Eigen::Vector<T, 3>(0.0, 1.0, 0.0)))
  EXPECT_MAT_EQ(
      (axisDirection<T, 3>(AxisDirection2::positive_z)),
      (Eigen::Vector<T, 3>(0.0, 0.0, 1.0)))
  EXPECT_MAT_EQ(
      (axisDirection<T, 3>(AxisDirection2::negative_x)),
      (Eigen::Vector<T, 3>(-1.0, 0.0, 0.0)))
  EXPECT_MAT_EQ(
      (axisDirection<T, 3>(AxisDirection2::negative_y)),
      (Eigen::Vector<T, 3>(0.0, -1.0, 0.0)))
  EXPECT_MAT_EQ(
      (axisDirection<T, 3>(AxisDirection2::negative_z)),
      (Eigen::Vector<T, 3>(0.0, 0.0, -1.0)))
}

TEST(coord_convention, axis_direction) {
  testAxisDirection<float>();
  testAxisDirection<double>();
}

template <typename T>
void testOrtho() {
  for (DeviceXyz convention : getAll(DeviceXyz())) {
    Eigen::Matrix<T, 3, 3> m = toConventionFromRdf<T>(convention);
    EXPECT_MAT_EQ(m * m.transpose(), (Eigen::Matrix<T, 3, 3>::Identity()))
    EXPECT_MAT_EQ(m.transpose() * m, (Eigen::Matrix<T, 3, 3>::Identity()))
  }
}

template <typename T>
void testRdf() {
  EXPECT_MAT_EQ(
      (toConventionFromRdf<T>(DeviceXyz::right_down_forward)),
      (Eigen::Matrix<T, 3, 3>::Identity()));

  {
    const Eigen::Matrix<T, 3, 3> flu_R_rdf =
        toConventionFromRdf<T>(DeviceXyz::forward_left_up);
    const Eigen::Vector<T, 3> forward_rdf(0.0, 0.0, 1.0);
    const Eigen::Vector<T, 3> down_rdf(0.0, 1.0, 0.0);
    EXPECT_MAT_EQ(
        (flu_R_rdf * forward_rdf),
        (Eigen::Vector<T, 3>(1.0, 0.0, 0.0))  // forward in FLU convention
    );
    EXPECT_MAT_EQ(
        (flu_R_rdf * down_rdf),
        (Eigen::Vector<T, 3>(0.0, 0.0, -1.0))  // down in FLU convention
    );
  }
  {
    const Eigen::Matrix<T, 3, 3> rub_R_rdf =
        toConventionFromRdf<T>(DeviceXyz::right_up_back);
    const Eigen::Vector<T, 3> forward_rdf(0.0, 0.0, 1.0);
    const Eigen::Vector<T, 3> down_rdf(0.0, 1.0, 0.0);
    EXPECT_MAT_EQ(
        (rub_R_rdf * forward_rdf),
        (Eigen::Vector<T, 3>(0.0, 0.0, -1.0))  // forward in RUB convention
    );
    EXPECT_MAT_EQ(
        (rub_R_rdf * down_rdf),
        (Eigen::Vector<T, 3>(0.0, -1.0, 0.0))  // down in RUB convention
    );
  }
}

TEST(coord_convention, ortho) {
  testOrtho<float>();
  testOrtho<double>();
}

TEST(coord_convention, rdf) {
  testRdf<float>();
  testRdf<double>();
}

}  // namespace pangolin
