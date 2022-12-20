// Copyright (c) farm-ng, inc. All rights reserved.

#include "test_data.h"

#include <pangolin/maths/conventions.h>
#include <pangolin/testing/eigen.h>

using namespace Eigen;
using namespace sophus;

namespace pangolin
{

template <typename T>
void testAxisDirection()
{
  CHECK_EIGEN_APPROX(
      (axisDirection<T, 3>(AxisDirection2::positive_x)),
      (Eigen::Vector<T, 3>(1.0, 0.0, 0.0)));

  CHECK_EIGEN_APPROX(
      (axisDirection<T, 3>(AxisDirection2::positive_y)),
      (Eigen::Vector<T, 3>(0.0, 1.0, 0.0)))
  CHECK_EIGEN_APPROX(
      (axisDirection<T, 3>(AxisDirection2::positive_z)),
      (Eigen::Vector<T, 3>(0.0, 0.0, 1.0)))
  CHECK_EIGEN_APPROX(
      (axisDirection<T, 3>(AxisDirection2::negative_x)),
      (Eigen::Vector<T, 3>(-1.0, 0.0, 0.0)))
  CHECK_EIGEN_APPROX(
      (axisDirection<T, 3>(AxisDirection2::negative_y)),
      (Eigen::Vector<T, 3>(0.0, -1.0, 0.0)))
  CHECK_EIGEN_APPROX(
      (axisDirection<T, 3>(AxisDirection2::negative_z)),
      (Eigen::Vector<T, 3>(0.0, 0.0, -1.0)))
}

template <typename T>
void testOrtho()
{
  for (DeviceXyz convention : getAll(DeviceXyz())) {
    Eigen::Matrix<T, 3, 3> m = toConventionFromRdf<T>(convention);
    CHECK_EIGEN_APPROX(m * m.transpose(), (Eigen::Matrix<T, 3, 3>::Identity()))
    CHECK_EIGEN_APPROX(m.transpose() * m, (Eigen::Matrix<T, 3, 3>::Identity()))
  }
}

template <typename T>
void testRdf()
{
  CHECK_EIGEN_APPROX(
      (toConventionFromRdf<T>(DeviceXyz::right_down_forward)),
      (Eigen::Matrix<T, 3, 3>::Identity()));

  {
    const Eigen::Matrix<T, 3, 3> flu_R_rdf =
        toConventionFromRdf<T>(DeviceXyz::forward_left_up);
    const Eigen::Vector<T, 3> forward_rdf(0.0, 0.0, 1.0);
    const Eigen::Vector<T, 3> down_rdf(0.0, 1.0, 0.0);
    CHECK_EIGEN_APPROX(
        (flu_R_rdf * forward_rdf),
        (Eigen::Vector<T, 3>(1.0, 0.0, 0.0))  // forward in FLU convention
    );
    CHECK_EIGEN_APPROX(
        (flu_R_rdf * down_rdf),
        (Eigen::Vector<T, 3>(0.0, 0.0, -1.0))  // down in FLU convention
    );
  }
  {
    const Eigen::Matrix<T, 3, 3> rub_R_rdf =
        toConventionFromRdf<T>(DeviceXyz::right_up_back);
    const Eigen::Vector<T, 3> forward_rdf(0.0, 0.0, 1.0);
    const Eigen::Vector<T, 3> down_rdf(0.0, 1.0, 0.0);
    CHECK_EIGEN_APPROX(
        (rub_R_rdf * forward_rdf),
        (Eigen::Vector<T, 3>(0.0, 0.0, -1.0))  // forward in RUB convention
    );
    CHECK_EIGEN_APPROX(
        (rub_R_rdf * down_rdf),
        (Eigen::Vector<T, 3>(0.0, -1.0, 0.0))  // down in RUB convention
    );
  }
}

TEST_CASE("coord_convention direction")
{
  testAxisDirection<float>();
  testAxisDirection<double>();
}

TEST_CASE("coord_convention ortho")
{
  testOrtho<float>();
  testOrtho<double>();
}

TEST_CASE("coord_convention RDF")
{
  testRdf<float>();
  testRdf<double>();
}

}  // namespace pangolin
