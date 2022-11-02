// Copyright (c) farm-ng, inc. All rights reserved.

#include <pangolin/testing/eigen.h>
#include <pangolin/maths/eigen_scalar_methods.h>

#include "test_data.h"

namespace pangolin {

TEST_CASE("scalar_methods, scalars") {
  auto const x = testVecs();

  for (double const a : x) {
    for (double const b : x) {
      CHECK(pangolin::min(a, b) == std::min(a, b));
      CHECK(pangolin::max(a, b) == std::max(a, b));
    }
  }
}

TEST_CASE("scalar_methods, eigen_vecs") {
  // const auto m3x4 = testMat<double,3,4>(testVec(), 100);
  // const auto m5x2 = testMat<double,5,2>(testVec(), 100);
  // const auto m1x1 = testMat<double,1,1>(testVec(), 100);
  // const auto m1x6 = testMat<double,1,1>(testVec(), 100);

  Eigen::Matrix2d x1, x2;
  x1 << 1, 2, 3, 4;
  x2 << 5, -4, 7, 1e7;

  {
    Eigen::Matrix2d y = pangolin::min(x1, x2);
    CHECK(y(0, 0) == 1);
    CHECK(y(0, 1) == -4);
    CHECK(y(1, 0) == 3);
    CHECK(y(1, 1) == 4);
  }
  {
    Eigen::Matrix2d y = pangolin::max(x1, x2);
    CHECK(y(0, 0) == 5);
    CHECK(y(0, 1) == 2);
    CHECK(y(1, 0) == 7);
    CHECK(y(1, 1) == 1e7);
  }
}

}  // namespace pangolin
