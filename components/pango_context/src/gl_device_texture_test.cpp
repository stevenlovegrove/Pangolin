#include <pangolin/render/device_texture.h>
#include <pangolin/testing/eigen.h>

using namespace pangolin;

TEST_CASE("smoke"){
    auto tex = DeviceTexture::Create();
}
