#include <pangolin/context/context.h>
#include <pangolin/gl/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/layer/all_layers.h>
#include <pangolin/video/video.h>

#include <pangolin/utils/file_notifier.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

// Forward declaration. See end of this file for the inline shader code.
extern const char* eg_shader;

struct ExampleCustomLayer : public Layer {
  ExampleCustomLayer(const std::filesystem::path& shader_path)
  {
    prog->reload({.sources = {{.origin = shader_path}}});
  }

  std::string name() const override { return "example"; }

  Size sizeHint() const override { return size_; }

  void renderIntoRegion(const Context& c, const RenderParams& p) override
  {
    c.setViewport(p.region);
    auto bind_prog = prog->bind();
    auto bind_vao = vao.bind();
    u_time = std::fmod(u_time.getValue() + 0.01, 1.0);
    PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
  }

  bool handleEvent(const Context&, const Event& e) override
  {
    if (std::holds_alternative<KeyboardEvent>(e.detail)) {
      prog->reload();
      return true;
    }
    return false;
  }

  GlVertexArrayObject vao = {};
  Shared<GlSlProgram> prog = GlSlProgram::Create({});
  GlUniform<float> u_time = {prog, "time"};
  Size size_ = {Parts{1}, Parts{1}};
};

int main(int argc, char** argv)
{
  auto notifier = FileChangeNotifier::create("/Users/stevenlovegrove/test_file.txt", [](const std::string& file){
    FARM_INFO("File changed '{}'", file);
  });

  // Keep the main thread running to monitor the file indefinitely
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

  return 0;
  PANGO_ENSURE(
      argc == 2,
      "Please provide one argument - the path to a full-screen shader.");
  const std::filesystem::path shader_path = argv[1];

  auto context = Context::Create({
      .title = "Pangolin Shader Reload",
      .window_size = {640, 480},
  });

  auto custom = Shared<ExampleCustomLayer>::make(shader_path);
  context->setLayout(custom);

  context->loop();
  return 0;
}
