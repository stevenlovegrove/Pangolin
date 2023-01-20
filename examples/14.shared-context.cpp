#include <pangolin/context/context.h>
#include <pangolin/gl/gl.h>
#include <pangolin/gui/all_layers.h>
#include <pangolin/image/image_io.h>
#include <pangolin/windowing/window.h>

using namespace pangolin;

struct RawGlLayer : public Layer {
  RawGlLayer(std::function<void()> user_func) : user_func(user_func) {}

  std::string name() const override { return "GL"; }

  Size sizeHint() const override { return {Parts{1.0}, Parts{1.0}}; }

  void renderIntoRegion(const Context& c, const RenderParams& p) override
  {
    c.setViewport(p.region);
    user_func();
  }

  static Shared<RawGlLayer> Create(std::function<void()> user_func)
  {
    return Shared<RawGlLayer>::make(user_func);
  }

  std::function<void()> user_func;
};

std::atomic<bool> keep_running = true;

void differentContextThread(Shared<WindowInterface> window, GlTexture& tex)
{
  window->MakeCurrent();
  PANGO_GL_CHECK();

  GlRenderBuffer rbo(tex.width, tex.height);
  PANGO_GL_CHECK();
  GlFramebuffer fbo(tex, rbo);
  PANGO_GL_CHECK();
  fbo.Bind();
  glViewport(0,0,tex.width, tex.height);

  int frame = 0;
  while (keep_running) {
    float r = (frame % 100) / 100.0;
    float g = (frame % 200) / 200.0;
    float b = (frame % 400) / 400.0;

    glClearColor(r, g, b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ++frame;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

int main(int /*argc*/, char** /*argv*/)
{
  auto context = Context::Create({
      .title = "Hello Pangolin World!",
  });

  GlTexture tex(100, 100);

    auto win2 = WindowInterface::Create(
        {.uri = ParseUri("default:[w=1,h=1,GL_PROFILE=LEGACY]//"),
         .shared_context = context->window()});
    std::thread other_context =
        std::thread(differentContextThread, win2, std::ref(tex));

//   GlRenderBuffer rbo(tex.width, tex.height);
//   PANGO_GL_CHECK();
//   GlFramebuffer fbo(tex, rbo);
//   PANGO_GL_CHECK();
//   fbo.Bind();

//   int frame = 0;

  context->window()->MakeCurrent();

  auto ui_image = DrawnImage::Create({});

  context->setLayout(ui_image);

  context->loop([&](){
      IntensityImage<> image;
      tex.Download(image);
      ui_image->image->update(image);
        return true;
  });

  keep_running = false;
    other_context.join();
  return 0;
}
