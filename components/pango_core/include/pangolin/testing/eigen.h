#pragma once

#include <catch2/catch_test_macros.hpp>
#include <sophus2/common/common.h>

#define CHECK_EIGEN_APPROX(A, B)                                               \
  CHECK(                                                                       \
      (A - B).squaredNorm() <                                                  \
      sophus2::kEpsilon<decltype((A - B).squaredNorm())>);

#define REQUIRE_EIGEN_APPROX(A, B)                                             \
  CHECK(                                                                       \
      (A - B).squaredNorm() <                                                  \
      sophus2::kEpsilon<decltype((A - B).squaredNorm())>);
