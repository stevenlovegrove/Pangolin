// Copyright (c) farm-ng, inc. All rights reserved.

#pragma once

#include <Eigen/Core>

#include <vector>

#define EXPECT_MAT_EQ(A, B)  \
  EXPECT_LT(                 \
      (A - B).squaredNorm(), \
      sophus::kEpsilon<decltype((A - B).squaredNorm())>);

namespace pangolin {

inline std::vector<double> testVecs() {
  // TODO: add more
  return {0.0, 98.1234, -9887.0, 1E5, 4E-3};
}

template <typename T, int kR, int kC>
std::vector<Eigen::Matrix<T, kR, kC>> testMats(
    std::vector<T> const& scalars, int const num_to_make) {
  std::vector<Eigen::Matrix<T, kR, kC>> ret;

  for (int k = 0; k < num_to_make; ++k) {
    Eigen::Matrix<T, kR, kC> mat;
    for (int r = 0; r < kR; ++r) {
      for (int c = 0; c < kC; ++c) {
        // pseudo random index
        int index = (k * 6131 + r * 2503 + c * 1409) % scalars.size();
        mat(r, c) = scalars[index];
      }
    }
    ret.push_back(mat);
  }
  return ret;
}

}  // namespace pangolin
