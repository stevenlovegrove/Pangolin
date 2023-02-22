#include <catch2/catch_test_macros.hpp>
#include <pangolin/layer/all_layers.h>
#include <pangolin/layer/layer_group.h>

namespace pangolin
{

TEST_CASE("computeLayoutConstraints, smoke")
{
  Shared<DrawLayer> default_draw_layer = pangolin::DrawLayer::Create({});
  LayerGroup group = makeLayerGroup(default_draw_layer);
  computeLayoutConstraints(group);
}
}  // namespace pangolin
