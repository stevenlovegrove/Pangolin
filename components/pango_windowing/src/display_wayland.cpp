#include <pangolin/platform.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/gl/colour.h>
#include <pangolin/gl/gldraw.h>
#include <pangolin/windowing/window.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <xkbcommon/xkbcommon.h>
#include <wayland-cursor.h>
#include <linux/input.h>
#include <sys/mman.h>

#include <mutex>
#include <string.h>
#include <unistd.h>
#include <cstdlib>

#include <xdg-shell-client-protocol.h>

#define WAYLAND_VERSION_GE(MAJ, MIN) WAYLAND_VERSION_MAJOR >= MAJ && WAYLAND_VERSION_MINOR >= MIN

// The "wl_array_for_each" C macro for C++
// https://github.com/libretro/RetroArch/blob/a9125fffaa981cab811ba6caf4d756fa6ef9a561/input/common/wayland_common.h#L50-L53
#define WL_ARRAY_FOR_EACH(pos, array, type) \
    for (pos = (type)(array)->data; \
         (const char *) pos < ((const char *) (array)->data + (array)->size); \
         (pos)++)

namespace pangolin {

namespace wayland {

static const std::map<enum xdg_toplevel_resize_edge, std::string> resize_cursor = {
    {XDG_TOPLEVEL_RESIZE_EDGE_NONE, "grabbing"},
    {XDG_TOPLEVEL_RESIZE_EDGE_TOP, "top_side"},
    {XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM, "bottom_side"},
    {XDG_TOPLEVEL_RESIZE_EDGE_LEFT, "left_side"},
    {XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT, "top_left_corner"},
    {XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT, "bottom_left_corner"},
    {XDG_TOPLEVEL_RESIZE_EDGE_RIGHT, "right_side"},
    {XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT, "top_right_corner"},
    {XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT, "bottom_right_corner"},
};

struct ButtonSurface {
    struct wl_surface *surface;
    struct wl_subsurface *subsurface;
    struct wl_egl_window *egl_window;
    EGLSurface egl_surface;
    EGLContext egl_context;
    EGLDisplay egl_display;
    const int32_t x, y;
    const uint width, height;
    pangolin::Colour colour;

    enum type {
        CLOSE = 100,
        MAXIMISE
    } function;

    ButtonSurface(wl_compositor* compositor, wl_subcompositor* subcompositor,
                  wl_surface* source, EGLDisplay egl_display, EGLConfig config,
                  int32_t x, int32_t y, uint width, uint height,
                  type fnct, pangolin::Colour colour
                  ) :
        egl_display(egl_display),
        x(x), y(y),
        width(width), height(height),
        colour(colour),
        function(fnct)
    {
        surface = wl_compositor_create_surface(compositor);
        subsurface = wl_subcompositor_get_subsurface(subcompositor, surface, source);
        egl_context = eglCreateContext (egl_display, config, EGL_NO_CONTEXT, nullptr);
        egl_window = wl_egl_window_create(surface, width, height);
        egl_surface = eglCreateWindowSurface(egl_display, config, (EGLNativeWindowType)egl_window, nullptr);
    }

    ~ButtonSurface() {
        if(egl_surface) eglDestroySurface(egl_display, egl_surface);
        if(egl_window)  wl_egl_window_destroy(egl_window);
        if(egl_context) eglDestroyContext(egl_display, egl_context);

        if(subsurface)  wl_subsurface_destroy(subsurface);
        if(surface)     wl_surface_destroy(surface);
    }

    void reposition(const int main_w) const {
        wl_subsurface_set_position(subsurface, main_w-x-width, y);
    }

    void draw() const {
        eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
        glClearColor(colour.r, colour.g, colour.b, colour.a);
        glClear(GL_COLOR_BUFFER_BIT);
        switch (function) {
        case CLOSE:
            glLineWidth(3);
            glColor3f(1, 1, 1);
            glBegin(GL_LINES);
            glVertex2f(-1, -1);
            glVertex2f(1, 1);
            glVertex2f(1, -1);
            glVertex2f(-1, 1);
            glEnd();
            break;
        case MAXIMISE:
            glLineWidth(2);
            glColor3f(0, 0, 0);
            glBegin(GL_LINE_LOOP);
            glVertex2f(-0.7f, -0.7f);
            glVertex2f(0.7f, -0.7f);
            glVertex2f(0.7f, 0.7f);
            glVertex2f(-0.7f, 0.7f);
            glEnd();
            glLineWidth(3);
            glBegin(GL_LINES);
            glVertex2f(+0.7f, +0.7f);
            glVertex2f(-0.7f, +0.7f);
            glEnd();
            break;
        }
        eglSwapInterval(egl_display, 0);
        eglSwapBuffers(egl_display, egl_surface);
    }
};

struct DecorationSurface {
    struct wl_surface *surface;
    struct wl_subsurface *subsurface;
    struct wl_egl_window *egl_window;
    EGLSurface egl_surface;
    EGLContext egl_context;
    EGLDisplay egl_display;
    pangolin::Colour colour;
    uint border_size;
    uint title_bar_size;

    enum xdg_toplevel_resize_edge function;

    DecorationSurface(wl_compositor* compositor, wl_subcompositor* subcompositor,
                      wl_surface* source, EGLDisplay egl_display, EGLConfig config,
                      const uint _border_size, const uint _title_bar_size,
                      enum xdg_toplevel_resize_edge type, pangolin::Colour _colour
                      ) :
        egl_display(egl_display),
        colour(_colour),
        border_size(_border_size),
        title_bar_size(_title_bar_size),
        function(type)
    {
        surface = wl_compositor_create_surface(compositor);
        subsurface = wl_subcompositor_get_subsurface(subcompositor, surface, source);
        egl_context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT, nullptr);
        egl_window = wl_egl_window_create(surface, 50, 50);
        egl_surface = eglCreateWindowSurface(egl_display, config, (EGLNativeWindowType)egl_window, nullptr);
    }

    ~DecorationSurface() {
        if(egl_surface) eglDestroySurface(egl_display, egl_surface);
        if(egl_window)  wl_egl_window_destroy(egl_window);
        if(egl_context) eglDestroyContext(egl_display, egl_context);

        if(subsurface)  wl_subsurface_destroy(subsurface);
        if(surface)     wl_surface_destroy(surface);
    }

    void calc_dim(const int main_w, const int main_h, int &x, int &y, int &w, int &h) const {
        // get position and dimension from type and main surface
        switch (function) {
        case XDG_TOPLEVEL_RESIZE_EDGE_NONE:
            x=0; y=-title_bar_size;
            w=main_w; h=title_bar_size;
            break;
        case XDG_TOPLEVEL_RESIZE_EDGE_TOP:
            x=0; y=-title_bar_size-border_size;
            w=main_w; h=border_size;
            break;
        case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM:
            x=0; y=main_h;
            w=main_w; h=border_size;
            break;
        case XDG_TOPLEVEL_RESIZE_EDGE_LEFT:
            x=-border_size; y=-title_bar_size;
            w=border_size; h=main_h+title_bar_size;
            break;
        case XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT:
            x=-border_size; y=-border_size-title_bar_size;
            w=border_size; h=border_size;
            break;
        case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT:
            x=-border_size; y=main_h;
            w=border_size; h=border_size;
            break;
        case XDG_TOPLEVEL_RESIZE_EDGE_RIGHT:
            x=main_w; y=-title_bar_size;
            w=border_size; h=main_h+title_bar_size;
            break;
        case XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT:
            x=main_w; y=-border_size-title_bar_size;
            w=border_size; h=border_size;
            break;
        case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT:
            x=main_w; y=main_h;
            w=border_size; h=border_size;
            break;
        }
    }

    void resize(const int main_w, const int main_h) const {
        int x=0, y=0, w=0, h=0;
        calc_dim(main_w, main_h, x, y, w, h);
        wl_subsurface_set_position(subsurface, x, y);
        wl_egl_window_resize(egl_window, w, h, 0, 0);
    }

    void draw() const {
        eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
        glClearColor(colour.r, colour.g, colour.b, colour.a);
        glClear(GL_COLOR_BUFFER_BIT);
        eglSwapInterval(egl_display, 0);
        eglSwapBuffers(egl_display, egl_surface);
    }
};

struct Decoration {
    Decoration(const uint border_size,
               const uint title_size,
               const pangolin::Colour colour,
               wl_compositor* compositor,
               wl_subcompositor* subcompositor,
               wl_surface* surface,
               EGLDisplay egl_display,
               EGLConfig config
               ) :
        border_size(border_size),
        title_size(title_size),
        colour(colour),
        egl_display(egl_display),
        compositor(compositor),
        subcompositor(subcompositor),
        surface(surface),
        config(config)
    { }

    void create() {
        // reserve memory to prevent that DecorationSurface's destructor gets
        // called by 'emplace_back'
        decorations.reserve(9);

        // title bar, 2D movement
        decorations.emplace_back(compositor, subcompositor, surface, egl_display, config, border_size, title_size, XDG_TOPLEVEL_RESIZE_EDGE_NONE, colour);

        // sides, 1D resizing
        decorations.emplace_back(compositor, subcompositor, surface, egl_display, config, border_size, title_size, XDG_TOPLEVEL_RESIZE_EDGE_LEFT, colour);
        decorations.emplace_back(compositor, subcompositor, surface, egl_display, config, border_size, title_size, XDG_TOPLEVEL_RESIZE_EDGE_RIGHT, colour);
        decorations.emplace_back(compositor, subcompositor, surface, egl_display, config, border_size, title_size, XDG_TOPLEVEL_RESIZE_EDGE_TOP, colour);
        decorations.emplace_back(compositor, subcompositor, surface, egl_display, config, border_size, title_size, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM, colour);

        // corners, 2D resizing
        decorations.emplace_back(compositor, subcompositor, surface, egl_display, config, border_size, title_size, XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT, colour);
        decorations.emplace_back(compositor, subcompositor, surface, egl_display, config, border_size, title_size, XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT, colour);
        decorations.emplace_back(compositor, subcompositor, surface, egl_display, config, border_size, title_size, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT, colour);
        decorations.emplace_back(compositor, subcompositor, surface, egl_display, config, border_size, title_size, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT, colour);

        // buttons
        buttons.reserve(2);
        buttons.emplace_back(compositor, subcompositor, decorations[0].surface, egl_display, config, 5, 1, button_width, button_height,  ButtonSurface::type::CLOSE, pangolin::Colour(0, 110/255.0, 182/255.0));
        buttons.emplace_back(compositor, subcompositor, decorations[0].surface, egl_display, config, 10+button_width, 1, button_width, button_height,  ButtonSurface::type::MAXIMISE, pangolin::Colour(1.0, 204/255.0f, 0));
    }

    void destroy() {
        decorations.clear();
        buttons.clear();
    }

    void resize(const int32_t width, const int32_t height) {
        for(const DecorationSurface &d : decorations) { d.resize(width, height); }
        for(const ButtonSurface &b : buttons) { b.reposition(width); }
    }

    void draw() {
        for(const DecorationSurface &d : decorations) { d.draw(); }
        for(const ButtonSurface &b : buttons) { b.draw(); }
    }

    void setTypeFromSurface(const wl_surface *surface) {
        for(const DecorationSurface &d : decorations) {
            if(d.surface==surface) {
                last_type = d.function;
                return;
            }
        }
        for(const ButtonSurface &b : buttons) {
            if(b.surface==surface) {
                last_type = b.function;
                return;
            }
        }
        // surface is not part of the window decoration
        last_type = -1;
    }

    const std::string getCursorForCurrentSurface() const {
        return resize_cursor.count((enum xdg_toplevel_resize_edge)last_type) ? resize_cursor.at((enum xdg_toplevel_resize_edge)last_type) : "left_ptr";
    }

    void toContentSize(const int window_w, const int window_h, int &content_w, int &content_h) {
        content_w = window_w - (2*border_size);
        content_h = window_h - (2*border_size + title_size);
    }

    void toWindowSize(const int content_w, const int content_h, int &window_w, int &window_h) {
        window_w = content_w + (2*border_size);
        window_h = content_h + (2*border_size + title_size);
    }

    std::vector<DecorationSurface> decorations;
    int last_type;

    std::vector<ButtonSurface> buttons;

    const uint border_size;
    const uint title_size;
    const pangolin::Colour colour;
    EGLDisplay egl_display;
    wl_compositor* compositor;
    wl_subcompositor* subcompositor;
    wl_surface* surface;
    EGLConfig config;

    static const uint button_width;
    static const uint button_height;
};

const uint Decoration::button_width = 25;
const uint Decoration::button_height = 15;

struct WaylandWindow;

struct WaylandDisplay {
    WaylandDisplay();

    ~WaylandDisplay();

    struct wl_display *wdisplay = nullptr;
    struct wl_registry *wregistry = nullptr;
    struct wl_compositor *wcompositor = nullptr;
    struct wl_subcompositor *wsubcompositor = nullptr;
    struct xdg_wm_base *xshell = nullptr;

    struct wl_seat *wseat = nullptr;
    struct wl_keyboard *wkeyboard = nullptr;
    struct wl_pointer *pointer = nullptr;

    // for cursor
    struct wl_shm *shm = nullptr;
    struct wl_cursor_theme *cursor_theme = nullptr;
    struct wl_surface *cursor_surface = nullptr;

    // xkbcommon
    struct xkb_context *xkb_context = nullptr;
    struct xkb_keymap *keymap = nullptr;
    struct xkb_state *xkb_state = nullptr;
    KeyModifierBitmask flags;

    std::vector<EGLConfig> egl_configs;
    EGLContext egl_context = nullptr;
    EGLDisplay egl_display = nullptr;

    static constexpr EGLint attribs[] = {
        EGL_RENDERABLE_TYPE , EGL_OPENGL_BIT,
        EGL_RED_SIZE        , 8,
        EGL_GREEN_SIZE      , 8,
        EGL_BLUE_SIZE       , 8,
        EGL_DEPTH_SIZE      , 24,
        EGL_STENCIL_SIZE    , 8,
        EGL_NONE
    };

    // assume a single window for now
    WaylandWindow* window;
};

constexpr EGLint WaylandDisplay::attribs[];

struct WaylandWindow : public WindowInterface
{
public:
    WaylandWindow(const int width, const int height, const std::string& title, std::shared_ptr<WaylandDisplay> display);

    ~WaylandWindow() override;

    void ShowFullscreen(const TrueFalseToggle on_off) override;

    void Move(const int x, const int y) override;

    void Resize(const unsigned int w, const unsigned int h) override;

    void MakeCurrent() override;

    void RemoveCurrent() override;

    void SwapBuffers() override;

    void ProcessEvents() override;

//private:
    std::shared_ptr<WaylandDisplay> display;

    EGLint width, height;
    bool is_fullscreen;
    bool is_maximised;
    // store floating window dimensions to restore to,
    // when returning from maximised or fullscreen
    int floating_width = 0, floating_height = 0;

    bool pressed = false;
    int lastx=0;
    int lasty=0;

    struct wl_surface *wsurface = nullptr;
    struct wl_egl_window *egl_window = nullptr;
    struct xdg_surface *xshell_surface = nullptr;
    struct xdg_toplevel *xshell_toplevel = nullptr;

    // we can only attach a buffer to a surface after it has been configured at least once
    bool configured = false;

    EGLSurface egl_surface = nullptr;

    std::shared_ptr<Decoration> decoration;
};

// map wayland ids to pangolin ids
static const std::map<uint,int> wl_button_ids = {
    {BTN_LEFT, 0},
    {BTN_MIDDLE, 1},
    {BTN_RIGHT, 2},
};

static const std::map<uint,KeyModifier> wl_key_mod_ids = {
    {KEY_LEFTSHIFT, KeyModifierShift},
    {KEY_RIGHTSHIFT, KeyModifierShift},
    {KEY_LEFTCTRL, KeyModifierCtrl},
    {KEY_RIGHTCTRL, KeyModifierCtrl},
    {KEY_LEFTALT, KeyModifierAlt},
    {KEY_RIGHTALT, KeyModifierAlt},
};

static const std::map<uint,int> wl_key_special_ids = {
    {KEY_F1, PANGO_KEY_F1},
    {KEY_F2, PANGO_KEY_F2},
    {KEY_F3, PANGO_KEY_F3},
    {KEY_F4, PANGO_KEY_F4},
    {KEY_F5, PANGO_KEY_F5},
    {KEY_F6, PANGO_KEY_F6},
    {KEY_F7, PANGO_KEY_F7},
    {KEY_F8, PANGO_KEY_F8},
    {KEY_F9, PANGO_KEY_F9},
    {KEY_F10, PANGO_KEY_F10},
    {KEY_F11, PANGO_KEY_F11},
    {KEY_F12, PANGO_KEY_F12},

    {KEY_LEFT, PANGO_KEY_LEFT},
    {KEY_UP, PANGO_KEY_UP},
    {KEY_RIGHT, PANGO_KEY_RIGHT},
    {KEY_DOWN, PANGO_KEY_DOWN},

    {KEY_PAGEUP, PANGO_KEY_PAGE_UP},
    {KEY_PAGEDOWN, PANGO_KEY_PAGE_DOWN},
    {KEY_HOME, PANGO_KEY_HOME},
    {KEY_END, PANGO_KEY_END},
    {KEY_INSERT, PANGO_KEY_INSERT},
};

static void handle_configure_toplevel(void *data, struct xdg_toplevel */*xdg_toplevel*/, int32_t width, int32_t height, struct wl_array *states) {

    const static uint min_width = 70;
    const static uint min_height = 70;

    WaylandWindow* const w = static_cast<WaylandWindow*>(data);

    // reset window states
    w->is_maximised = w->is_fullscreen = false;

    // set new window states
    const enum xdg_toplevel_state *state;
    WL_ARRAY_FOR_EACH(state, states, const enum xdg_toplevel_state*) {
        switch (*state) {
        case XDG_TOPLEVEL_STATE_MAXIMIZED:
            w->is_maximised = true;
            break;
        case XDG_TOPLEVEL_STATE_FULLSCREEN:
            w->is_fullscreen = true;
            break;
        default:
            break;
        }
    }

    // set proposed dimensions, this includes the entire window with decorations
    int restore_w = width;
    int restore_h = height;

    // restore from stored floating state if not specified
    if (restore_w==0) restore_w = w->floating_width;
    if (restore_h==0) restore_h = w->floating_height;

    // use initially provided dimensions if floating was never set,
    // these are in the "content" dimensions
    if (restore_w==0) restore_w = w->width;
    if (restore_h==0) restore_h = w->height;

    if(!w->is_fullscreen && (width!=0 && height!=0)) {
        // has decoration
        w->decoration->toContentSize(restore_w, restore_h, w->width, w->height);
        w->width = std::max(w->width, int(min_width));
        w->height = std::max(w->height, int(min_height));
    }
    else {
        w->width = restore_w;
        w->height = restore_h;
    }

    // store the current state as floating state,
    // if we are neither maximised nor fullscreen
    if (!w->is_maximised && !w->is_fullscreen) {
        w->floating_width = w->width;
        w->floating_height = w->height;
    }
}

static void handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    WaylandWindow* const w = static_cast<WaylandWindow*>(data);

    // resize main surface
    wl_egl_window_resize(w->egl_window, w->width, w->height, 0, 0);

    // resize all decoration elements
    w->decoration->resize(w->width, w->height);

    // notify Panglin views about resized area
    w->ResizeSignal(WindowResizeEvent{w->width, w->height});

    xdg_surface_ack_configure(xdg_surface, serial);

    w->configured = true;
}

static const struct xdg_surface_listener shell_surface_listener = {
    handle_configure,
};

static void handle_toplevel_close(void *data, struct xdg_toplevel */*xdg_toplevel*/) {
    static_cast<WaylandWindow*>(data)->CloseSignal();
}

static const struct xdg_toplevel_listener toplevel_listener = {
    .configure = handle_configure_toplevel,
    .close = handle_toplevel_close,
};

static void xdg_wm_base_ping(void */*data*/, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener shell_listener = {
    .ping = xdg_wm_base_ping
};

static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t /*sx*/, wl_fixed_t /*sy*/) {
    WaylandDisplay* const d = static_cast<WaylandDisplay*>(data);
    WaylandWindow* const w = d->window;
    w->decoration->setTypeFromSurface(surface);

    const std::string cursor = w->decoration->getCursorForCurrentSurface();

    const auto image = wl_cursor_theme_get_cursor(d->cursor_theme, cursor.c_str())->images[0];
    wl_pointer_set_cursor(pointer, serial, d->cursor_surface, image->hotspot_x, image->hotspot_y);
    wl_surface_attach(d->cursor_surface, wl_cursor_image_get_buffer(image), 0, 0);
    wl_surface_damage(d->cursor_surface, 0, 0, image->width, image->height);
    wl_surface_commit(d->cursor_surface);
}

static void pointer_handle_leave(void *data, struct wl_pointer */*pointer*/, uint32_t /*serial*/, struct wl_surface */*surface*/) {
    WaylandDisplay* const d = static_cast<WaylandDisplay*>(data);
    d->window->pressed = false;
}

static void pointer_handle_motion(void *data, struct wl_pointer */*pointer*/, uint32_t /*time*/, wl_fixed_t sx, wl_fixed_t sy) {
    WaylandDisplay* const d = static_cast<WaylandDisplay*>(data);
    WaylandWindow* const w = d->window;

    w->lastx=wl_fixed_to_int(sx);
    w->lasty=wl_fixed_to_int(sy);
    if(w->pressed) {
        w->MouseMotionSignal(MouseMotionEvent{{float(w->lastx), float(w->lasty), d->flags}});
    }
    else {
        w->PassiveMouseMotionSignal(MouseMotionEvent{{float(w->lastx), float(w->lasty), d->flags}});
    }
}

static void pointer_handle_button(void *data, struct wl_pointer */*wl_pointer*/, uint32_t serial, uint32_t /*time*/, uint32_t button, uint32_t state) {
    WaylandDisplay* const d = static_cast<WaylandDisplay*>(data);
    WaylandWindow* const w = d->window;

    if(w->decoration->last_type<0) {
        // input goes to pangoling view
        if(!wl_button_ids.count(button))
            return;

        w->pressed = (state==WL_POINTER_BUTTON_STATE_PRESSED);
        w->MouseSignal(MouseEvent{{float(w->lastx), float(w->lasty), d->flags}, wl_button_ids.at(button), w->pressed});
    }
    else {
        // input goes to window decoration
        // resizing using window decoration
        if((button==BTN_LEFT) && (state==WL_POINTER_BUTTON_STATE_PRESSED)) {
            switch (w->decoration->last_type) {
            case XDG_TOPLEVEL_RESIZE_EDGE_NONE:
                xdg_toplevel_move(w->xshell_toplevel, d->wseat, serial);
                break;
            case XDG_TOPLEVEL_RESIZE_EDGE_TOP:
            case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM:
            case XDG_TOPLEVEL_RESIZE_EDGE_LEFT:
            case XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT:
            case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT:
            case XDG_TOPLEVEL_RESIZE_EDGE_RIGHT:
            case XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT:
            case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT:
                xdg_toplevel_resize(w->xshell_toplevel, d->wseat, serial, w->decoration->last_type);
                break;
            case ButtonSurface::type::CLOSE:
                w->CloseSignal();
                break;
            case ButtonSurface::type::MAXIMISE:
                if(w->is_maximised) {
                    xdg_toplevel_unset_maximized(w->xshell_toplevel);
                }
                else {
                    xdg_toplevel_set_maximized(w->xshell_toplevel);
                }

                break;
            }
        }
    }
}

static void pointer_handle_axis(void *data, struct wl_pointer */*wl_pointer*/, uint32_t /*time*/, uint32_t axis, wl_fixed_t value) {
    WaylandDisplay* const d = static_cast<WaylandDisplay*>(data);
    WaylandWindow* const w = d->window;

    const float v = wl_fixed_to_double(value);
    float dx = 0, dy = 0;

    switch (axis) {
    case REL_X: dy = v; break;   // up, down
    case REL_Y: dx = v; break;   // left, right
    }

    w->SpecialInputSignal(SpecialInputEvent({
        {float(w->lastx), float(w->lasty), d->flags},
        InputSpecialScroll, {dx, dy, 0, 0}
    }));
}

#if WAYLAND_VERSION_GE(1,12)

static void pointer_handle_frame(void */*data*/, struct wl_pointer */*wl_pointer*/) { }

static void pointer_handle_axis_source(void */*data*/, struct wl_pointer */*wl_pointer*/, uint32_t /*axis_source*/) { }

static void pointer_handle_axis_stop(void */*data*/, struct wl_pointer */*wl_pointer*/, uint32_t /*time*/, uint32_t /*axis*/) { }

static void pointer_handle_axis_discrete(void */*data*/, struct wl_pointer */*wl_pointer*/, uint32_t /*axis*/, int32_t /*discrete*/) { }

#endif

static const struct wl_pointer_listener pointer_listener = {
    pointer_handle_enter,
    pointer_handle_leave,
    pointer_handle_motion,
    pointer_handle_button,
    pointer_handle_axis,
#if WAYLAND_VERSION_GE(1,12)
    pointer_handle_frame,
    pointer_handle_axis_source,
    pointer_handle_axis_stop,
    pointer_handle_axis_discrete,
#endif
};

static void keyboard_handle_keymap(void *data, struct wl_keyboard */*keyboard*/, uint32_t format, int fd, uint32_t size) {
    if ((!data) || (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)) {
        std::cerr << "wrong keymap format, got " << format << ", expected WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1" << std::endl;
        close(fd);
        return;
    }

    WaylandDisplay* const d = static_cast<WaylandDisplay*>(data);

    char *keymap_string = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (keymap_string == MAP_FAILED) {
        std::cerr << "keymap mmap failed: " << std::string(std::strerror(errno)) << std::endl;
        close(fd);
        return;
    }
    xkb_keymap_unref(d->keymap);
    d->keymap = xkb_keymap_new_from_string(d->xkb_context, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(keymap_string, size);
    close(fd);
    xkb_state_unref(d->xkb_state);
    d->xkb_state = xkb_state_new(d->keymap);
}

static void keyboard_handle_enter(void *data, struct wl_keyboard */*keyboard*/, uint32_t /*serial*/, struct wl_surface */*surface*/, struct wl_array */*keys*/) {
    static_cast<WaylandDisplay*>(data)->flags = {};
}

static void keyboard_handle_leave(void */*data*/, struct wl_keyboard */*keyboard*/, uint32_t /*serial*/, struct wl_surface */*surface*/) { }

static void keyboard_handle_key(void *data, struct wl_keyboard */*keyboard*/, uint32_t /*serial*/, uint32_t /*time*/, uint32_t key, uint32_t state) {
    WaylandDisplay* const d = static_cast<WaylandDisplay*>(data);
    WaylandWindow* const w = d->window;

    // modifier keys
    if(wl_key_mod_ids.count(key)) {
        if(state==WL_KEYBOARD_KEY_STATE_PRESSED) {
            d->flags |= wl_key_mod_ids.at(key);
        }
        else if (state==WL_KEYBOARD_KEY_STATE_RELEASED) {
            // NOTE: "d->flags &= ~wl_key_mod_ids.at(key);" does not work here
            d->flags.set(wl_key_mod_ids.at(key), false);
        }
        return;
    }

    // character and special keys
    int pango_key = -1;
    if(wl_key_special_ids.count(key)) {
        // special keys
        pango_key = PANGO_SPECIAL + wl_key_special_ids.at(key);
    }
    else {
        // character keys
        const uint32_t utf32 = xkb_state_key_get_utf32(d->xkb_state, key+8);
        // filter non-ASCII
        if(utf32>0 && utf32<=127) {
            pango_key = int(utf32);
        }
    }

    if(pango_key>0) {
        w->KeyboardSignal(KeyboardEvent{{float(w->lastx), float(w->lasty), d->flags}, uint8_t(pango_key), state==WL_KEYBOARD_KEY_STATE_PRESSED});
    }
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard */*keyboard*/, uint32_t /*serial*/, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    WaylandDisplay* const d = static_cast<WaylandDisplay*>(data);

    xkb_state_update_mask(d->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

#if WAYLAND_VERSION_GE(1,12)

static void keyboard_handle_repeat_info(void */*data*/, struct wl_keyboard */*wl_keyboard*/, int32_t /*rate*/, int32_t /*delay*/) { }

#endif

static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap,
    keyboard_handle_enter,
    keyboard_handle_leave,
    keyboard_handle_key,
    keyboard_handle_modifiers,
#if WAYLAND_VERSION_GE(1,12)
    keyboard_handle_repeat_info,
#endif
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t caps1) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    enum wl_seat_capability caps;
    caps = (enum wl_seat_capability)caps1;
    if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
        w->wkeyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(w->wkeyboard, &keyboard_listener, data);
    } else {
        wl_keyboard_destroy(w->wkeyboard);
        w->wkeyboard = nullptr;
    }
    if (caps & WL_SEAT_CAPABILITY_POINTER) {
        w->pointer = wl_seat_get_pointer(seat);
        w->cursor_surface = wl_compositor_create_surface(w->wcompositor);
        wl_pointer_add_listener(w->pointer, &pointer_listener, data);
    } else {
        wl_pointer_destroy(w->pointer);
        w->pointer = nullptr;
    }
}

static void seat_handle_name(void */*data*/, struct wl_seat */*wl_seat*/, const char */*name*/) { }

static const struct wl_seat_listener seat_listener = {
    seat_handle_capabilities,
    seat_handle_name,
};

static void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        w->wcompositor = reinterpret_cast<wl_compositor*> (wl_registry_bind(registry, id, &wl_compositor_interface, std::min<uint32_t>(version,4)));
    }
    else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
        w->wsubcompositor = static_cast<wl_subcompositor*>(wl_registry_bind(registry, id, &wl_subcompositor_interface, std::min<uint32_t>(version,1)));
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        w->xshell = reinterpret_cast<xdg_wm_base*> (wl_registry_bind(registry, id, &xdg_wm_base_interface, std::min<uint32_t>(version,3)));
    }
    else if (strcmp(interface, wl_seat_interface.name) == 0) {
        w->wseat = reinterpret_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, std::min<uint32_t>(version,5)));
        wl_seat_add_listener(w->wseat, &seat_listener, data);
    }
    else if (strcmp(interface, wl_shm_interface.name) == 0) {
        w->shm = static_cast<wl_shm*>(wl_registry_bind(registry, id, &wl_shm_interface, std::min<uint32_t>(version,1)));
        w->cursor_theme = wl_cursor_theme_load(nullptr, 16, w->shm);
    }
}

static void global_registry_remover(void */*data*/, struct wl_registry */*registry*/, uint32_t /*id*/) { }

static const struct wl_registry_listener wregistry_listener = {
    global_registry_handler,
    global_registry_remover
};

WaylandDisplay::WaylandDisplay() {
    xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    wdisplay = wl_display_connect(nullptr);
    if (wdisplay == nullptr) {
        throw std::runtime_error("Cannot connect to Wayland compositor!");
    }

    wregistry = wl_display_get_registry(wdisplay);
    wl_registry_add_listener(wregistry, &wregistry_listener, this);

    wl_display_roundtrip(wdisplay);

    egl_display = eglGetDisplay((EGLNativeDisplayType)wdisplay);
    if(!egl_display) {
        std::cerr << "Failed to open EGL display" << std::endl;
    }

    EGLint major, minor;
    if(eglInitialize(egl_display, &major, &minor)==EGL_FALSE) {
        std::cerr << "EGL init failed" << std::endl;
    }

    if(eglBindAPI(EGL_OPENGL_API)==EGL_FALSE) {
        std::cerr << "EGL bind failed" << std::endl;
    }

    EGLint count;
    eglGetConfigs(egl_display, nullptr, 0, &count);

    egl_configs.resize(count);

    EGLint numConfigs;
    eglChooseConfig(egl_display, attribs, egl_configs.data(), count, &numConfigs);

    egl_context = eglCreateContext(egl_display, egl_configs[0], EGL_NO_CONTEXT, nullptr);

    if(xshell==nullptr) {
        throw std::runtime_error("No Wayland shell available!");
    }

    xdg_wm_base_add_listener(xshell, &shell_listener, this);

    wl_display_roundtrip(wdisplay);
}

WaylandDisplay::~WaylandDisplay() {
    // cleanup EGL
    if(egl_context) eglDestroyContext(egl_display, egl_context);
    if(egl_display) eglTerminate(egl_display);

    // cleanup Wayland
    if(wkeyboard)       wl_keyboard_destroy(wkeyboard);
    if(pointer)         wl_pointer_destroy(pointer);
    if(cursor_surface)  wl_surface_destroy(cursor_surface);
    if(cursor_theme)    wl_cursor_theme_destroy(cursor_theme);
    if(shm)             wl_shm_destroy(shm);
    if(wseat)           wl_seat_destroy(wseat);
    if(xshell)          xdg_wm_base_destroy(xshell);
    if(wsubcompositor)  wl_subcompositor_destroy(wsubcompositor);
    if(wcompositor)     wl_compositor_destroy(wcompositor);
    if(wregistry)       wl_registry_destroy(wregistry);
    if(wdisplay)        wl_display_disconnect(wdisplay);

    if(xkb_context) xkb_context_unref(xkb_context);
    if(xkb_state)   xkb_state_unref(xkb_state);
    if(keymap)      xkb_keymap_unref(keymap);
}

WaylandWindow::WaylandWindow(const int w, const int h,
                             const std::string& title,
                             std::shared_ptr<WaylandDisplay> display)
    : display(display)
{
    wsurface = wl_compositor_create_surface(display->wcompositor);

    display->window = this;

    width = w;
    height = h;

    egl_window = wl_egl_window_create(wsurface, w, h);
    if(!egl_window) {
        std::cerr << "Cannot create EGL window" << std::endl;
    }

    egl_surface = eglCreateWindowSurface(display->egl_display, display->egl_configs[0], (EGLNativeWindowType)egl_window, nullptr);
    if (egl_surface == EGL_NO_SURFACE) {
        std::cerr << "Cannot create EGL surface" << std::endl;
    }

    xshell_surface = xdg_wm_base_get_xdg_surface(display->xshell, wsurface);
    xdg_surface_add_listener(xshell_surface, &shell_surface_listener, this);
    xshell_toplevel = xdg_surface_get_toplevel(xshell_surface);
    xdg_toplevel_add_listener(xshell_toplevel, &toplevel_listener, this);
    xdg_toplevel_set_title(xshell_toplevel, title.c_str());
    xdg_toplevel_set_app_id(xshell_toplevel, title.c_str());

    // construct window decoration
    const pangolin::Colour grey(0.5f, 0.5f, 0.5f);
    decoration = std::unique_ptr<Decoration>(new Decoration(5, 20, grey, display->wcompositor, display->wsubcompositor, wsurface, display->egl_display, display->egl_configs[0]));
    decoration->create();
    decoration->resize(width, height);

    wl_surface_commit(wsurface);

    wl_display_roundtrip(display->wdisplay);
    wl_display_roundtrip(display->wdisplay);

    // wait for the first configure event
    while (!configured) {
        wl_display_dispatch(display->wdisplay);
    }
}

WaylandWindow::~WaylandWindow() {
    if(decoration)  decoration->destroy();

    // cleanup EGL
    if(egl_surface) eglDestroySurface(display->egl_display, egl_surface);
    if(egl_window)  wl_egl_window_destroy(egl_window);

    // cleanup Wayland
    if(xshell_surface)  xdg_surface_destroy(xshell_surface);
    if(xshell_toplevel) xdg_toplevel_destroy(xshell_toplevel);

    if(wsurface)        wl_surface_destroy(wsurface);
}

void WaylandWindow::MakeCurrent() {
    eglMakeCurrent(display->egl_display, egl_surface, egl_surface, display->egl_context);
}

void WaylandWindow::RemoveCurrent() {
    eglMakeCurrent(display->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void WaylandWindow::ShowFullscreen(const TrueFalseToggle on_off) {
    switch (on_off) {
    case TrueFalseToggle::False:
        decoration->create();
        xdg_toplevel_unset_fullscreen(xshell_toplevel);
        break;
    case TrueFalseToggle::True:
        decoration->destroy();
        xdg_toplevel_set_fullscreen(xshell_toplevel, nullptr);
        break;
    case TrueFalseToggle::Toggle:
        ShowFullscreen(TrueFalseToggle(!is_fullscreen));
        break;
    }

    wl_display_sync(display->wdisplay);
}

void WaylandWindow::Move(const int /*x*/, const int /*y*/) { }

void WaylandWindow::Resize(const unsigned int /*w*/, const unsigned int /*h*/) { }

void WaylandWindow::ProcessEvents() { }

void WaylandWindow::SwapBuffers() {
    eglSwapBuffers(display->egl_display, egl_surface);

    // draw all decoration elements
    decoration->draw();

    MakeCurrent();

    wl_display_dispatch(display->wdisplay);
}


std::unique_ptr<WindowInterface> CreateWaylandWindowAndBind(const std::string window_title, const int w, const int h, const std::string /*display_name*/, const bool /*double_buffered*/, const int /*sample_buffers*/, const int /*samples*/) {

    try{
        return std::make_unique<WaylandWindow>(w, h, window_title, std::make_shared<WaylandDisplay>());
    }
    catch(const std::runtime_error&) {
        // return null pointer for fallback to X11
        return nullptr;
    }
}

} // namespace wayland

PANGOLIN_REGISTER_FACTORY(WaylandWindow)
{
  struct WaylandWindowFactory : public TypedFactoryInterface<WindowInterface> {
    std::map<std::string,Precedence> Schemes() const override
    {
      return {{"wayland",10}, {"linux",9}, {"default",90}};
    }

    const char* Description() const override
    {
      return "Use X11 native window";
    }

    ParamSet Params() const override
    {
      return {{
          {"window_title","window","Title of application Window"},
          {"w","640","Requested window width"},
          {"h","480","Requested window height"},
          {"display_name","","The display name to open the window on"},
          {"double_buffered","true","Whether the window should be double buffered"},
          {"sample_buffers","1",""},
          {"samples","1",""},
      }};
    }

    std::unique_ptr<WindowInterface> Open(const Uri& uri) override {

      const std::string window_title = uri.Get<std::string>("window_title", "window");
      const int w = uri.Get<int>("w", 640);
      const int h = uri.Get<int>("h", 480);
      const std::string display_name = uri.Get<std::string>("display_name", "");
      const bool double_buffered = uri.Get<bool>("double_buffered", true);
      const int sample_buffers = uri.Get<int>("sample_buffers", 1);
      const int samples = uri.Get<int>("samples", 1);
      return std::unique_ptr<WindowInterface>(wayland::CreateWaylandWindowAndBind(window_title, w, h, display_name, double_buffered, sample_buffers, samples));
    }

    virtual ~WaylandWindowFactory() { }
  };

    return FactoryRegistry::I()->RegisterFactory<WindowInterface>(std::make_shared<WaylandWindowFactory>());
}

} // namespace pangolin
