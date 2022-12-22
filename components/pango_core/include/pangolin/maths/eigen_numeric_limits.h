// Copyright (c) farm-ng, inc. All rights reserved.

#pragma once

#include "eigen_concepts.h"

#include <Eigen/Core>

#include <limits>

namespace pangolin
{

// Ideally we could use Eigen's own trait system but it appears to be missing
// some important traits such as lowest(), min() and max().
//
// template<typename T>
// using MultiDimLimits = Eigen::NumTraits<T>;

// Probably frowned upon, but we'll inherit from the std::library traits
// for ordinary scalars etc.
template <typename T>
struct MultiDimLimits : public std::numeric_limits<T> {
};

template <EigenDenseType TT>
class MultiDimLimits<TT>
{
  public:
  using TScalar = typename TT::Scalar;
  static constexpr int kRows = TT::RowsAtCompileTime;
  static constexpr int kCols = TT::ColsAtCompileTime;

  static constexpr bool is_specialized = true;
  static constexpr bool has_infinity =
      std::numeric_limits<TScalar>::has_infinity;
  static constexpr bool has_quiet_NaN =
      std::numeric_limits<TScalar>::has_quiet_NaN;
  static constexpr bool has_signaling_NaN =
      std::numeric_limits<TScalar>::has_signaling_NaN;
  /// ... plus a bunch more if we need them

  static TT lowest()
  {
    return TT::Constant(std::numeric_limits<TScalar>::lowest());
  };
  static TT min() { return TT::Constant(std::numeric_limits<TScalar>::min()); };
  static TT max() { return TT::Constant(std::numeric_limits<TScalar>::max()); };
  static TT epsilon()
  {
    return TT::Constant(std::numeric_limits<TScalar>::epsilon());
  };
  /// ... plus a bunch more if we need them
};

}  // namespace pangolin
