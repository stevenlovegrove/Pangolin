// Copyright (c) farm-ng, inc. All rights reserved.

#include "eigen_scalar_methods.h"

#include "test_data.h"

#include <gtest/gtest.h>

namespace pangolin {

TEST(scalar_methods, scalars) {
  auto const x = testVecs();

  for (double const a : x) {
    for (double const b : x) {
      EXPECT_EQ(pangolin::min(a, b), std::min(a, b));
      EXPECT_EQ(pangolin::max(a, b), std::max(a, b));
    }
  }
}

TEST(scalar_methods, eigen_vecs) {
  // const auto m3x4 = testMat<double,3,4>(testVec(), 100);
  // const auto m5x2 = testMat<double,5,2>(testVec(), 100);
  // const auto m1x1 = testMat<double,1,1>(testVec(), 100);
  // const auto m1x6 = testMat<double,1,1>(testVec(), 100);

  Eigen::Matrix2d x1, x2;
  x1 << 1, 2, 3, 4;
  x2 << 5, -4, 7, 1e7;

  {
    Eigen::Matrix2d y = pangolin::min(x1, x2);
    EXPECT_EQ(y(0, 0), 1);
    EXPECT_EQ(y(0, 1), -4);
    EXPECT_EQ(y(1, 0), 3);
    EXPECT_EQ(y(1, 1), 4);
  }
  {
    Eigen::Matrix2d y = pangolin::max(x1, x2);
    EXPECT_EQ(y(0, 0), 5);
    EXPECT_EQ(y(0, 1), 2);
    EXPECT_EQ(y(1, 0), 7);
    EXPECT_EQ(y(1, 1), 1e7);
  }
}

}  // namespace pangolin
