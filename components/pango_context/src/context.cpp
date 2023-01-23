#include <fmt/format.h>
#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/gl/gl_type_info.h>
#include <pangolin/gl/glplatform.h>
#include <pangolin/gui/interactive.h>
#include <pangolin/gui/layer_group.h>
#include <pangolin/utils/reverse_iterable.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/windowing/window.h>

namespace pangolin
{

struct EngineImpl : public Engine {
};

Shared<Engine> Engine::singleton()
{
  static Shared<Engine> global = Shared<EngineImpl>::make();
  return global;
}

void renderIntoRegionImpl(
    const Context& context, const Layer::RenderParams& p,
    const LayerGroup& group)
{
  if (group.layer) {
    group.layer->renderIntoRegion(context, {.region = group.cached_.region});
  }
  if (group.grouping == LayerGroup::Grouping::stacked) {
    // Back-to-front for blending / overlays
    for (const auto& child : reverse(group.children)) {
      renderIntoRegionImpl(context, p, child);
    }
  } else if (group.grouping == LayerGroup::Grouping::tabbed) {
    PANGO_ENSURE(group.selected_tab < group.children.size());
    renderIntoRegionImpl(context, p, group.children[group.selected_tab]);
  } else {
    for (const auto& child : group.children) {
      renderIntoRegionImpl(context, p, child);
    }
  }
}

void renderIntoRegion(
    const Context& context, const Layer::RenderParams& p,
    const LayerGroup& group)
{
  computeLayoutConstraints(group);
  computeLayoutRegion(group, p.region);
  renderIntoRegionImpl(context, p, group);
}

// Find layer to receive event based on pointer location
// Return the layer which processed the event.
//
// Pre-condition: layer positions have been computed by a previous call to
// render
std::shared_ptr<Layer> giveEventToLayers(
    const Context& context, Interactive::Event event, const LayerGroup& group)
{
  const auto r = group.cached_.region;
  const Eigen::Vector2i winpos = event.pointer_pos.pos_window.cast<int>();

  if (r.contains(winpos)) {
    event.pointer_pos.region = r;

    if (group.layer && group.layer->handleEvent(context, event)) {
      // event handled, stop dfs
      return group.layer;
    }

    // see if child nodes want it
    for (const auto& child : group.children) {
      auto layer = giveEventToLayers(context, event, child);
      if (layer) return layer;
    }
  }
  return nullptr;
}

// This is kinda dumb - we look through all nodes even though
// we know the active_layer already. We do this just so we can
// find the cached region so we can send it through
bool giveEventToActiveLayer(
    const Context& context, Interactive::Event event, const LayerGroup& group,
    const std::shared_ptr<Layer>& active_layer)
{
  PANGO_ENSURE(active_layer);

  const auto r = group.cached_.region;
  event.pointer_pos.region = r;

  if (group.layer == active_layer) {
    group.layer->handleEvent(context, event);
  }

  // see if child nodes want it
  for (const auto& child : group.children) {
    const bool found =
        giveEventToActiveLayer(context, event, child, active_layer);
    if (found) return true;
  }

  return false;
}

namespace
{

Uri parseWindowUri(const Context::Params& params)
{
  if (const char* env_params = std::getenv("PANGOLIN_WINDOW_URI")) {
    // Environment-specified params take precedent
    return ParseUri(env_params);
  }
  return ParseUri(fmt::format(
      "{}:[window_title={},w={},h={},GL_PROFILE={}]//", params.window_engine,
      params.title, params.window_size.width, params.window_size.height,
      "3.2 CORE"));
}

}  // namespace

struct ContextImpl : public Context {
  // TODO: Convert Window to use new factory idiom directly
  ContextImpl(const Context::Params& params) :
      size_(params.window_size),
      window_(Window::Create({.uri = parseWindowUri(params)}))
  {
    window()->ResizeSignal.connect([this](const WindowResizeEvent& e) {
      size_.width = e.width;
      size_.height = e.height;
      // commenting these in will trigger render during
      // resize, but there can be artifacts
      // window()->SwapBuffers();
      // drawPanels();
    });
    window()->MouseSignal.connect(&ContextImpl::mouseEvent, this);
    window()->MouseMotionSignal.connect(&ContextImpl::mouseMotionEvent, this);
    window()->SpecialInputSignal.connect(&ContextImpl::specialInputEvent, this);
    window()->KeyboardSignal.connect(&ContextImpl::keyboardEvent, this);

    window()->MakeCurrent();
    window()->ProcessEvents();
    glInit();
  }

  void glInit()
  {
    // All functions will maintain these as an invariant on return.
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
  }

  void setViewport(
      const Region2I& bounds,
      ImageXy image_convention = Conventions::global().image_xy) const override
  {
    const auto gl_bounds = regionGlFromConvention(bounds, image_convention);
    const Eigen::Array2i pos = gl_bounds.min();
    const Eigen::Array2i size =
        gl_bounds.range().array() + Eigen::Array2i(1, 1);
    glViewport(pos.x(), pos.y(), size.x(), size.y());
    glScissor(pos.x(), pos.y(), size.x(), size.y());
  }

  // TODO: push these enums up into window interface to avoid conversion
  static std::optional<PointerButton> toInteractiveButton(int window_button)
  {
    switch (window_button) {
      case MouseButton::MouseButtonLeft:
        return PointerButton::primary;
      case MouseButton::MouseButtonMiddle:
        return PointerButton::tertiary;
      case MouseButton::MouseButtonRight:
        return PointerButton::secondary;
      case MouseButton::MouseWheelUp:    // scroll up
      case MouseButton::MouseWheelDown:  // scroll down
        return std::nullopt;
      // case 5: return PointerButton::back;
      // case 6: return PointerButton::forward;
      default:
        PANGO_WARN("Unexpected input button, {}.", window_button);
        return std::nullopt;
    }
  }

  // TODO: push these enums up into window interface to avoid conversion
  static Interactive::ModifierKeyStatus toInteractiveModifierKey(
      KeyModifierBitmask mod)
  {
    Interactive::ModifierKeyStatus inkey;
    if (mod & KeyModifierShift) inkey.set(ModifierKey::shift);
    if (mod & KeyModifierCtrl) inkey.set(ModifierKey::ctrl);
    if (mod & KeyModifierAlt) inkey.set(ModifierKey::alt_option);
    if (mod & KeyModifierCmd) inkey.set(ModifierKey::win_cmd_meta);
    if (mod & KeyModifierFnc) inkey.set(ModifierKey::fn);
    return inkey;
  }

  void mouseEvent(MouseEvent e)
  {
    modifier_active_ = toInteractiveModifierKey(e.key_modifiers);

    if (e.button == MouseWheelUp || e.button == MouseWheelDown) {
      const float delta = (e.button == MouseWheelDown ? -1.0f : 1.0f);
      Interactive::Event layer_event = {
          .pointer_pos = WindowPosition{.pos_window = {e.x, e.y}},
          .modifier_active = modifier_active_,
          .detail =
              Interactive::ScrollEvent{// .pan = Eigen::Array2d(0.0, delta)
                                       .zoom = delta / 100.0}};
      dispatchLayerEvent(layer_event);
    } else {
      auto maybe_button = toInteractiveButton(e.button);
      if (!maybe_button) return;

      button_active_.set(*maybe_button, e.pressed);

      Interactive::Event layer_event = {
          .pointer_pos = WindowPosition{.pos_window = {e.x, e.y}},
          .modifier_active = modifier_active_,
          .detail = Interactive::PointerEvent{
              .action =
                  e.pressed ? PointerAction::down : PointerAction::click_up,
              .button = *maybe_button,
              .button_active = button_active_,
          }};
      dispatchLayerEvent(
          layer_event,
          e.pressed ? ActiveLayerAction::capture : ActiveLayerAction::release);
    }
  }

  void mouseMotionEvent(MouseMotionEvent e)
  {
    modifier_active_ = toInteractiveModifierKey(e.key_modifiers);

    Interactive::Event layer_event = {
        .pointer_pos = WindowPosition{.pos_window = {e.x, e.y}},
        .modifier_active = modifier_active_,
        .detail = Interactive::PointerEvent{
            .action = PointerAction::drag,
            .button_active = button_active_,
        }};
    dispatchLayerEvent(layer_event);
  }

  void specialInputEvent(SpecialInputEvent e)
  {
    modifier_active_ = toInteractiveModifierKey(e.key_modifiers);

    if (e.inType == InputSpecialScroll) {
      Interactive::Event layer_event = {
          .pointer_pos = WindowPosition{.pos_window = {e.x, e.y}},
          .modifier_active = modifier_active_,
          .detail = Interactive::ScrollEvent{.pan = {e.p[0], e.p[1]}}};
      dispatchLayerEvent(layer_event);
    } else if (e.inType == InputSpecialZoom) {
      Interactive::Event layer_event = {
          .pointer_pos = WindowPosition{.pos_window = {e.x, e.y}},
          .modifier_active = modifier_active_,
          .detail = Interactive::ScrollEvent{.zoom = e.p[0]}};
      dispatchLayerEvent(layer_event);
    }
  }

  void keyboardEvent(KeyboardEvent e)
  {
    modifier_active_ = toInteractiveModifierKey(e.key_modifiers);

    if (e.key == 27) {  // escape
      should_run = false;
    } else if (e.key == 9) {  // tab
      window_->ShowFullscreen(TrueFalseToggle::Toggle);
    } else {
      Interactive::Event layer_event = {
          .pointer_pos = WindowPosition{.pos_window = {e.x, e.y}},
          .modifier_active = modifier_active_,
          .detail =
              Interactive::KeyboardEvent{.key = e.key, .pressed = e.pressed}};
      dispatchLayerEvent(layer_event);
      // PANGO_INFO("Unprocessed keypress, {}", int(e.key) );
    }
  }

  enum class ActiveLayerAction { ignore, capture, release };

  void dispatchLayerEvent(
      const Interactive::Event& src,
      ActiveLayerAction active_layer_action = ActiveLayerAction::ignore)
  {
    if (active_layer_) {
      giveEventToActiveLayer(*this, src, layout_, active_layer_);
      if (active_layer_action == ActiveLayerAction::release) {
        active_layer_ = nullptr;
      }
    } else {
      auto layer = giveEventToLayers(*this, src, layout_);
      if (active_layer_action == ActiveLayerAction::capture) {
        active_layer_ = layer;
      }
    }
  }

  Shared<Window> window() override { return window_; }

  void setLayout(const LayerGroup& layout) override { layout_ = layout; }

  const LayerGroup& layout() const override { return layout_; }

  LayerGroup& layout() override { return layout_; }

  ImageSize size() const override { return size_; }

  void drawPanels()
  {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Region2I region = Region2I::empty();
    region.extend({0, 0});
    region.extend({size_.width, size_.height});

    renderIntoRegion(*this, {.region = region}, layout_);
  }

  void loop(std::function<bool(void)> loop_function) override
  {
    should_run = true;

    auto close_connection =
        window()->CloseSignal.connect([&]() { should_run = false; });

    while (should_run && loop_function()) {
      drawPanels();
      window()->SwapBuffers();
      window()->ProcessEvents();
    }

    should_run = false;
  }

  bool isRunning() const override { return should_run; }

  sophus::IntensityImage<> read(
      Region2I bounds, Attachment attachment,
      ImageXy image_axis_convention) const override
  {
    using namespace sophus;
    const auto gl_bounds =
        regionGlFromConvention(bounds, image_axis_convention);
    const Eigen::Array2i imsize = bounds.range().array() + Eigen::Array2i(1, 1);
    const bool is_depth = attachment == Attachment::depth;

    const RuntimePixelType pixel_type =
        is_depth ? RuntimePixelType::fromTemplate<float>()
                 : RuntimePixelType::fromTemplate<sophus::Pixel3U8>();

    const auto maybe_gl_pixel_type = glTypeInfo(pixel_type);
    const GlFormatInfo gl_pixel_type = SOPHUS_UNWRAP(maybe_gl_pixel_type);

    sophus::IntensityImage<> image(ImageSize(imsize[0], imsize[1]), pixel_type);

    glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    glDrawBuffer(GL_FRONT);

    glReadPixels(
        gl_bounds.min().x(), gl_bounds.min().y(), imsize.x(), imsize.y(),
        is_depth ? GL_DEPTH_COMPONENT : gl_pixel_type.gl_base_format,
        gl_pixel_type.gl_type, const_cast<uint8_t*>(image.rawPtr()));
    return image;
  }

  private:
  Region2I regionGlFromConvention(
      Region2I bounds, ImageXy axis_convention) const
  {
    if (axis_convention == ImageXy::right_up) {
      // Same as OpenGL
      return bounds;
    } else if (axis_convention == ImageXy::right_down) {
      // Reverse image Y
      const Eigen::Array2i pos = bounds.min();
      const Eigen::Array2i size = bounds.range().array() + Eigen::Array2i(1, 1);
      int y = size_.height - pos.y() - size.y();
      return Region2I::fromMinMax(
          {pos.x(), y}, {pos.x() + size.x() - 1, y + size.y() - 1});
    } else {
      PANGO_UNREACHABLE();
    }
  }

  ImageSize size_;
  Shared<Window> window_;
  LayerGroup layout_;
  std::shared_ptr<Layer> active_layer_;
  volatile std::atomic<bool> should_run;

  Interactive::PointerButtonStatus button_active_ = {};
  Interactive::ModifierKeyStatus modifier_active_ = {};
};

PANGO_CREATE(Context) { return Shared<ContextImpl>::make(p); }

}  // namespace pangolin
