#include <catch2/catch_test_macros.hpp>
#include <sophus/common/common.h>

#define CHECK_EIGEN_APPROX(A, B)                                               \
  CHECK(                                                                       \
      (A - B).squaredNorm() <                                                  \
      sophus::kEpsilon<decltype((A - B).squaredNorm())>);

#define REQUIRE_EIGEN_APPROX(A, B)                                             \
  CHECK(                                                                       \
      (A - B).squaredNorm() <                                                  \
      sophus::kEpsilon<decltype((A - B).squaredNorm())>);
