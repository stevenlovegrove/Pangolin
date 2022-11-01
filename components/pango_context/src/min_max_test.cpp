// Copyright (c) farm-ng, inc. All rights reserved.

#include <pangolin/testing/eigen.h>
#include <pangolin/maths/min_max.h>
#include <sophus/image/image.h>

namespace pangolin {

#if 0
template <typename T>
void typeChecks() {
  // Check default construction
  MinMax<T> mm;
  CHECK(mm.min() == std::numeric_limits<T>::max());
  CHECK(mm.max() == std::numeric_limits<T>::lowest());

  mm = MinMax<T>(10, 22);
  CHECK(mm == MinMax<T>(10, 22));
  mm.extend(11);
  CHECK(mm == MinMax<T>(10, 22));
  mm.extend(190);
  CHECK(mm == MinMax<T>(10, 190));
  mm.extend(7);
  CHECK(mm == MinMax<T>(7, 190));

  mm.extend(MinMax<T>(70, 200));
  CHECK(mm == MinMax<T>(7, 200));

  mm.extend(MinMax<T>(6, 201));
  CHECK(mm == MinMax<T>(6, 201));

  CHECK(mm != MinMax<T>(5, 201));
  CHECK(mm != MinMax<T>(6, 200));
}

TEST_CASE("minmax, scalar_test") {
  typeChecks<double>();
  typeChecks<float>();
  typeChecks<int16_t>();
  typeChecks<uint8_t>();
  typeChecks<size_t>();
  MinMax<double> mm(-1.0);
}

TEST_CASE("minmax, image_reduction") {
  {
    sophus::MutImage<double> img({10, 10});
    for (int y = 0; y < img.height(); ++y) {
      for (int x = 0; x < img.height(); ++x) {
        img.uncheckedMut(x, y) = x + y * img.width();
      }
    }
    auto min_max = finiteMinMax(img);
    CHECK(
        min_max == MinMax<double>(0.0, img.width() * img.height() - 1.0));
  }
}
#endif

}  // namespace pangolin
