#include <pangolin/factory/factory_registry.h>
#include <pangolin/windowing/window.h>

// clang-format(off)
#include <EGL/egl.h>
#ifdef Success
#undef Success
#endif
// clang-format(on)

namespace pangolin
{

namespace headless
{

class EGLDisplayHL
{
  public:
  EGLDisplayHL(int const width, int const height);

  ~EGLDisplayHL();

  void swap();

  void makeCurrent();

  void removeCurrent();

  private:
  EGLSurface egl_surface;
  EGLContext egl_context;
  EGLDisplay egl_display;

  static constexpr EGLint attribs[] = {
      EGL_SURFACE_TYPE,
      EGL_PBUFFER_BIT,
      EGL_RENDERABLE_TYPE,
      EGL_OPENGL_BIT,
      EGL_RED_SIZE,
      8,
      EGL_GREEN_SIZE,
      8,
      EGL_BLUE_SIZE,
      8,
      EGL_ALPHA_SIZE,
      8,
      EGL_DEPTH_SIZE,
      24,
      EGL_STENCIL_SIZE,
      8,
      EGL_NONE};
};

constexpr EGLint EGLDisplayHL::attribs[];

struct HeadlessWindow : public WindowInterface {
  HeadlessWindow(int const width, int const height);

  ~HeadlessWindow() override;

  void ShowFullscreen(const TrueFalseToggle on_off) override;

  void Move(int const x, int const y) override;

  void Resize(unsigned int const w, unsigned int const h) override;

  void MakeCurrent() override;

  void RemoveCurrent() override;

  void SwapBuffers() override;

  void ProcessEvents() override;

  EGLDisplayHL display;
};

EGLDisplayHL::EGLDisplayHL(int const width, int const height)
{
  egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (!egl_display) {
    std::cerr << "Failed to open EGL display" << std::endl;
  }

  EGLint major, minor;
  if (eglInitialize(egl_display, &major, &minor) == EGL_FALSE) {
    std::cerr << "EGL init failed" << std::endl;
  }

  if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
    std::cerr << "EGL bind failed" << std::endl;
  }

  EGLint count;
  eglGetConfigs(egl_display, nullptr, 0, &count);

  std::vector<EGLConfig> egl_configs(count);

  EGLint numConfigs;
  eglChooseConfig(egl_display, attribs, egl_configs.data(), count, &numConfigs);

  egl_context =
      eglCreateContext(egl_display, egl_configs[0], EGL_NO_CONTEXT, nullptr);

  const EGLint pbufferAttribs[] = {
      EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE,
  };
  egl_surface =
      eglCreatePbufferSurface(egl_display, egl_configs[0], pbufferAttribs);
  if (egl_surface == EGL_NO_SURFACE) {
    std::cerr << "Cannot create EGL surface" << std::endl;
  }
}

EGLDisplayHL::~EGLDisplayHL()
{
  if (egl_context) eglDestroyContext(egl_display, egl_context);
  if (egl_surface) eglDestroySurface(egl_display, egl_surface);
  if (egl_display) eglTerminate(egl_display);
}

void EGLDisplayHL::swap() { eglSwapBuffers(egl_display, egl_surface); }

void EGLDisplayHL::makeCurrent()
{
  eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
}

void EGLDisplayHL::removeCurrent()
{
  eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

HeadlessWindow::HeadlessWindow(int const w, int const h) : display(w, h) {}

HeadlessWindow::~HeadlessWindow() {}

void HeadlessWindow::MakeCurrent() { display.makeCurrent(); }

void HeadlessWindow::RemoveCurrent() { display.removeCurrent(); }

void HeadlessWindow::ShowFullscreen(const TrueFalseToggle) {}

void HeadlessWindow::Move(int const /*x*/, int const /*y*/) {}

void HeadlessWindow::Resize(unsigned int const /*w*/, unsigned int const /*h*/)
{
}

void HeadlessWindow::ProcessEvents() {}

void HeadlessWindow::SwapBuffers()
{
  display.swap();
  MakeCurrent();
}

}  // namespace headless

PANGOLIN_REGISTER_FACTORY(HeadlessWindow)
{
  struct HeadlessWindowFactory : public TypedFactoryInterface<WindowInterface> {
    std::map<std::string, Precedence> Schemes() const override
    {
      return {{"egl", 10}, {"nogui", 10}, {"headless", 10}, {"none", 10}};
    }
    char const* Description() const override { return "Headless GL Buffer"; }
    ParamSet Params() const override
    {
      return {{
          {"w", "640", "Requested buffer width"},
          {"h", "480", "Requested buffer height"},
          {"window_title", "main", "Title (Unused)"},
          {PARAM_GL_PROFILE, "Ignored for now"},
      }};
    }
    std::unique_ptr<WindowInterface> Open(Uri const& uri) override
    {
      return std::unique_ptr<WindowInterface>(new headless::HeadlessWindow(
          uri.Get<int>("w", 640), uri.Get<int>("h", 480)));
    }

    virtual ~HeadlessWindowFactory() {}
  };

  return FactoryRegistry::I()->RegisterFactory<WindowInterface>(
      std::make_shared<HeadlessWindowFactory>());
}

}  // namespace pangolin
