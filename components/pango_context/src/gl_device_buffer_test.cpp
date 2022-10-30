#include <pangolin/render/device_buffer.h>
#include <pangolin/testing/eigen.h>

using namespace pangolin;

TEST_CASE("smoke"){
    auto tex = DeviceBuffer::Create({DeviceBuffer::Kind::Texture});
}
