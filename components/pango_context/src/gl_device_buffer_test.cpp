#include <pangolin/render/device_buffer.h>
#include <pangolin/testing/eigen.h>

using namespace pangolin;

TEST_CASE("smoke"){
    auto bo = DeviceBuffer::Create({DeviceBuffer::Kind::VertexAttributes});
}
