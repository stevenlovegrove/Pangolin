#include <pangolin/gl/glplatform.h>
#include <pangolin/layer/layer.h>

namespace pangolin
{

struct ClearLayerImpl : public Layer {
  enum class Action { clear_depth, clear_color, clear_both };

  ClearLayerImpl(GLbitfield clear_mask) : clear_mask_(clear_mask) {}

  std::string name() const override { return "-"; }
  Size sizeHint() const override { return {Parts{0}, Parts{0}}; }

  void renderIntoRegion(const Context&, const RenderParams&) override
  {
    glClear(clear_mask_);
  }

  GLbitfield clear_mask_;
};

Shared<Layer> Layer::ClearZ()
{
  static auto instance = Shared<ClearLayerImpl>::make(GL_DEPTH_BUFFER_BIT);
  return instance;
}

Shared<Layer> Layer::ClearColor()
{
  static auto instance = Shared<ClearLayerImpl>::make(GL_COLOR_BUFFER_BIT);
  return instance;
}

Shared<Layer> Layer::Clear()
{
  static auto instance =
      Shared<ClearLayerImpl>::make(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  return instance;
}

}  // namespace pangolin
