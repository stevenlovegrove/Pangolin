#include <pangolin/utils/logging.h>
#include <catch2/catch_test_macros.hpp>

using namespace pangolin;

TEST_CASE("smoke"){
    PANGO_DEBUG("test warning");
    PANGO_INFO("test info");
    PANGO_UNIMPLEMENTED("test unimplemented");
    PANGO_WARN("test warning");
    PANGO_ERROR("test error");
    // PANGO_FATAL("test FATAL");

    PANGO_WARN("test warning1");
    PANGO_WARN("test warning2");
    PANGO_WARN("test warning3");
    PANGO_INFO("starting loop");

    for(int i=0; i < 10; ++i) {
        PANGO_WARN("test warning1");
        PANGO_WARN("test warning2");
        PANGO_WARN("test warning3");
    }

}
