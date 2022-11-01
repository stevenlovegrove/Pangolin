#pragma once

#include <array>
#include <pangolin/utils/shared.h>
#include <pangolin/context/engine.h>
#include <pangolin/gui/render_layer_group.h>
#include <sophus/image/image_size.h>

namespace pangolin
{

class WindowInterface;
struct RenderLayerGroup;
struct RenderLayer;

////////////////////////////////////////////////////////////////////
/// Represents the pangolin context for one Window or Window-like
/// offscreen buffer. A 'graphics' context is likely one-to-one with
/// this object, but that is an implementation detail and may not
/// always be true. For example a particular implementation may
/// internally share one graphics context between multiple Pangolin
/// Contexts
///
struct Context : std::enable_shared_from_this<Context>
{
    using Window = WindowInterface;
    using ImageSize = sophus::ImageSize;

    virtual ~Context() {}

    // Return the implementation window (onscreen or offscreen)
    // backing this context
    virtual Shared<Window> window() = 0;

    // Enter the blocking event loop that will process window events
    // such as user input and manage which elements are drawn within
    // the window. After each rendered frame, the user loop_function
    // is invoked within the same gui thread. The loop is terminated
    // if the user function returns false, or a termination signal
    // is received either programatically or via the window close
    // symbol.
    virtual void loop(std::function<bool(void)> loop_function) = 0;

    // Convenience method for looping without a user function
    inline void loop() { loop([](){return true;}); }

    // Specify the Panels which will make up the drawing canvas via
    // a RenderLayerGroup object - a nested tree of Panels with a layout
    // specification at each node.
    //
    // If a layout is already set, it will be replaced by layout.
    // If an end-user modifies the layout (through the GUI) at
    // runtime, it's changes can be seen through the getLayout()
    // method.
    //
    // Once detached, Panels will not automatically interact with
    // the Context. Uers may safely hold onto unused PanelGroups
    // and restore them via setLayout to quickly reconfigure the
    // window.
    virtual void setLayout(const RenderLayerGroup& layout) = 0;

    // Convenience method to create a window with only one panel
    virtual void setLayout(const Shared<RenderLayer>& panel) = 0;

    // Return the current RenderLayerGroup layout - this may have been
    // customized at runtime by the end-user.
    // TODO: provide a method to serialize RenderLayerGroup for easily
    //       saving layouts
    virtual RenderLayerGroup getLayout() const = 0;

    struct Params {
        std::string title = "Pangolin App";
        ImageSize window_size = {1024, 768};
        std::string window_engine = Engine::singleton()->defaults.window_engine;
        Engine::Profile profile = Engine::singleton()->defaults.profile;
        Shared<RenderLayerGroup> layout = {};
    };
    static Shared<Context> Create(Params p);
};

}
