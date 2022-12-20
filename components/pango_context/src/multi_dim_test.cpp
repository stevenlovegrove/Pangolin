#include <catch2/catch_test_macros.hpp>
#include <pangolin/maths/multi_dim.h>
#include <pangolin/utils/fmt.h>

using namespace pangolin;

TEST_CASE("multi_dim, static")
{
  {
    using Arr = MultiDimArray<int, 3, 2, 2>;
    Arr m;
    static_assert(MultiDimArray<int, 3, 2, 2>::kPackIntoStruct == true);
    static_assert(m.data.size() == 3 * 2 * 2);
  }
  {
    using Arr = MultiDimArray<int, 3, 2, 6>;
    Arr m;
    static_assert(Arr::kPackIntoStruct == false);
  }

  // println("{}", static_volume.data.size());
}
