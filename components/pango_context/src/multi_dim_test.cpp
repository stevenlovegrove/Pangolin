#include <pangolin/utils/fmt.h>
#include <pangolin/maths/multi_dim.h>
#include <catch2/catch_test_macros.hpp>

using namespace pangolin;

template<typename T>
using Image = MultiDimArray<T,Dynamic,Dynamic>;


TEST_CASE("multi_dim, static") {
    {
        MultiDimArray<int,3,2,2> m;
        static_assert(MultiDimArray<int,3,2,2>::kPackIntoStruct == true);

        // static_assert(m.data.size() == 3*2*2);
        CHECK(m.data.size() == 3*2*2);
    }
    {
        MultiDimArray<int,3,2,2> m;

    }

    // println("{}", static_volume.data.size());


}
