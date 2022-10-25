// Copyright (c) farm-ng, inc. All rights reserved.

#pragma once

#include "eigen_numeric_limits.h"
#include "eigen_scalar_methods.h"

#include <sophus/image/image_view.h>

namespace pangolin {

namespace {
template <typename T>
concept Differencable = requires(T a, T b) {
  b - a;
};
}  // namespace

// This works a lot like Eigens::AlignedBox, but supports
// TPixel as a scalar or vector element for use with generic
// image reductions etc
// The range of values is represented by min,max inclusive.
template <typename TPixel>
class MinMax {
 public:
  MinMax() = default;
  MinMax(MinMax const&) = default;
  MinMax(MinMax&&) = default;
  MinMax& operator=(MinMax const&) = default;

  MinMax(TPixel const& p) : min_max{p, p} {}

  MinMax(TPixel const& p1, TPixel const& p2) : min_max{p1, p1} { extend(p2); }

  TPixel const& min() const { return min_max[0]; }
  TPixel const& max() const { return min_max[1]; }

  TPixel clamp(TPixel const& x) const {
    return max(min(x, min_max[1]), min_max[0]);
  }

  bool contains(TPixel const& x) const {
    return allTrue(eval(min() <= x)) && allTrue(eval(x <= max()));
  }

  // Only applicable if minmax object is valid()
  auto range() requires Differencable<TPixel> { return max() - min(); }

  MinMax<TPixel>& extend(MinMax const& o) {
    min_max[0] = pangolin::min(min(), o.min());
    min_max[1] = pangolin::max(max(), o.max());
    return *this;
  }

  MinMax<TPixel>& extend(TPixel const& p) {
    min_max[0] = pangolin::min(min(), p);
    min_max[1] = pangolin::max(max(), p);
    return *this;
  }

  template <typename To>
  MinMax<To> cast() const {
    return MinMax<To>(pangolin::cast<To>(min()), pangolin::cast<To>(max()));
  }

  bool valid() const {
    return min_max[0] != MultiDimLimits<TPixel>::max() &&
           min_max[1] != MultiDimLimits<TPixel>::lowest();
  }

 private:
  // invariant that min_max[0] <= min_max[1]
  // or min_max is as below when uninitialized
  std::array<TPixel, 2> min_max = {
      MultiDimLimits<TPixel>::max(), MultiDimLimits<TPixel>::lowest()};
};

namespace details {
template <class TScalar>
class Cast<MinMax<TScalar>> {
 public:
  template <typename To>
  static MinMax<To> impl(MinMax<TScalar> const& v) {
    return v.template cast<To>();
  }
};
}  // namespace details

template <typename T>
bool operator==(MinMax<T> const& lhs, MinMax<T> const& rhs) {
  return lhs.min() == rhs.min() && lhs.max() == rhs.max();
}

template <typename Tpixel>
MinMax<Tpixel> finiteMinMax(sophus::ImageView<Tpixel> const& image) {
  return image.reduce(
      [](Tpixel v, auto& min_max) {
        if (isFinite(v)) min_max.extend(v);
      },
      MinMax<Tpixel>{});
}

template <typename Tpixel>
inline MinMax<Eigen::Vector2i> minMaxImageCoordsInclusive(
    sophus::ImageView<Tpixel> const& image, int border = 0) {
  // e.g. 10x10 image has valid values [0, ..., 9] in both dimensions
  // a border of 2 would make valid range [2, ..., 7]
  return MinMax<Eigen::Vector2i>(Eigen::Vector2i(border, border))
      .extend(Eigen::Vector2i(
          image.width() - border - 1, image.height() - border - 1));
}

}  // namespace pangolin
