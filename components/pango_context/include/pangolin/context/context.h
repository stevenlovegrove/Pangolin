#pragma once

#include <pangolin/context/engine.h>
#include <pangolin/gui/layer_group.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/maths/region.h>
#include <pangolin/utils/shared.h>
#include <sophus/image/dyn_image_types.h>
#include <sophus/image/image_size.h>

#include <array>

namespace pangolin
{

class WindowInterface;
struct Layer;

////////////////////////////////////////////////////////////////////
/// Represents the pangolin context for one Window or Window-like offscreen
/// buffer. A 'graphics' context is likely one-to-one with this object, but that
/// is an implementation detail and may not always be true. For example a
/// particular implementation may internally share one graphics context between
/// multiple Pangolin Contexts
///
struct Context : std::enable_shared_from_this<Context> {
  using Window = WindowInterface;
  using ImageSize = sophus::ImageSize;
  enum class Attachment {
    depth,
    color,  //...
  };

  virtual ~Context() {}

  // Return the implementation window (onscreen or offscreen) backing this
  // context
  virtual Shared<Window> window() = 0;

  // Enter the blocking event loop that will process window events such as user
  // input and manage which elements are drawn within the window. After each
  // rendered frame, the user loop_function is invoked within the same gui
  // thread. The loop is terminated if the user function returns false, or a
  // termination signal is received either programatically or via the window
  // close symbol.
  virtual void loop(std::function<bool(void)> loop_function) = 0;

  // Convenience method for looping without a user function
  inline void loop()
  {
    loop([]() { return true; });
  }

  // Returns true if the context is engaged in its loop function and has not
  // been asked to exit.
  virtual bool isRunning() const = 0;

  // Return the current LayerGroup layout - this may have been
  // customized at runtime by the end-user.
  // TODO: provide a method to serialize LayerGroup for easily
  //       saving layouts
  virtual const LayerGroup& layout() const = 0;

  // Specify the Layers which will make up the drawing canvas via a LayerGroup
  // object - a nested tree of Panels with a layout specification at each node.
  //
  // If a layout is already set, it will be replaced by layout. If an end-user
  // modifies the layout (through the GUI) at runtime, it's changes can be seen
  // through the getLayout() method.
  //
  // Once detached, Layers will not automatically interact with the Context.
  // Users may safely hold onto unused PanelGroups and restore them via
  // setLayout to quickly reconfigure the window.
  virtual void setLayout(const LayerGroup& layout) = 0;

  // Convenience method to accept anything convertable to a LayourGroup
  template <LayerGroupConvertable T>
  void setLayout(const T& object)
  {
    setLayout(makeLayerGroup(object));
  }

  // Return size of the internal (renderable area) of the window in pixels. This
  // is the window size, not the current viewport size
  virtual ImageSize size() const = 0;

  // Set the viewport region describing the destination and coordinate system
  // for future render operations. region is specified in pixel units of the
  // window/buffer managed by the context, with origin respecting the
  // window_convention parameter.
  virtual void setViewport(
      const Region2I& region,
      ImageXy window_convention = Conventions::global().image_xy) const = 0;

  // Read from the graphics device the specified buffer associated with this
  // context. For example, you can use this to read back an image to save, or to
  // query the rendered depth for user interaction.
  virtual sophus::IntensityImage<> read(
      Region2I region =
          Region2I::empty(),  // empty region will return full buffer
      Attachment attachment = Attachment::color,
      ImageXy image_axis_convention = Conventions::global().image_xy) const = 0;

  struct Params {
    std::string title = "Pangolin App";
    ImageSize window_size = {1024, 768};
    std::string window_engine = Engine::singleton()->defaults.window_engine;
    Engine::Profile profile = Engine::singleton()->defaults.profile;
  };
  static Shared<Context> Create(Params p);
};

}  // namespace pangolin
