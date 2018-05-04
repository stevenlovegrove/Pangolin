#include <pangolin/platform.h>
#include <pangolin/display/display.h>
#include <pangolin/display/display_internal.h>
#include <pangolin/factory/factory_registry.h>

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

#define WAYLAND_VERSION_GE(MAJ, MIN) WAYLAND_VERSION_MAJOR >= MAJ && WAYLAND_VERSION_MINOR >= MIN

namespace pangolin {

extern __thread PangolinGl* context;

namespace wayland {

struct WaylandDisplay {
    WaylandDisplay(const int width, const int height, const std::string title = "");

    ~WaylandDisplay();

    struct wl_display *wdisplay = nullptr;
    struct wl_registry *wregistry = nullptr;
    struct wl_compositor *wcompositor = nullptr;
    struct wl_surface *wsurface = nullptr;
    struct wl_egl_window *egl_window = nullptr;
    struct wl_shell *wshell = nullptr;
    struct wl_shell_surface *wshell_surface = nullptr;

    struct wl_seat *wseat = nullptr;
    struct wl_keyboard *wkeyboard = nullptr;
    struct wl_pointer *pointer = nullptr;

    // for cursor
    struct wl_shm *shm = nullptr;
    struct wl_cursor_theme *cursor_theme = nullptr;
    struct wl_cursor *default_cursor = nullptr;
    struct wl_surface *cursor_surface = nullptr;

    // xkbcommon
    struct xkb_context *xkb_context = nullptr;
    struct xkb_keymap *keymap = nullptr;
    struct xkb_state *xkb_state = nullptr;

    bool pressed = false;
    int lastx=0;
    int lasty=0;

    std::vector<EGLConfig> egl_configs;
    EGLSurface egl_surface;
    EGLContext egl_context;
    EGLDisplay egl_display;

    EGLint width, height;
    static constexpr EGLint attribs[] = {
        EGL_RENDERABLE_TYPE , EGL_OPENGL_BIT,
        EGL_RED_SIZE        , 8,
        EGL_GREEN_SIZE      , 8,
        EGL_BLUE_SIZE       , 8,
        EGL_ALPHA_SIZE      , 8,
        EGL_DEPTH_SIZE      , 24,
        EGL_STENCIL_SIZE    , 8,
        EGL_NONE
    };
};

constexpr EGLint WaylandDisplay::attribs[];

struct WaylandWindow : public PangolinGl
{
    WaylandWindow(const int width, const int height, std::unique_ptr<WaylandDisplay> display);

    ~WaylandWindow() override;

    void ToggleFullscreen() override;

    void Move(const int x, const int y) override;

    void Resize(const unsigned int w, const unsigned int h) override;

    void MakeCurrent() override;

    void SwapBuffers() override;

    void ProcessEvents() override;

    std::unique_ptr<WaylandDisplay> display;
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

static void pointer_handle_enter(void *data, struct wl_pointer */*pointer*/, uint32_t serial, struct wl_surface */*surface*/, wl_fixed_t /*sx*/, wl_fixed_t /*sy*/) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    struct wl_buffer *buffer;
    struct wl_cursor_image *image;

    image = w->default_cursor->images[0];
    buffer = wl_cursor_image_get_buffer(image);
    wl_pointer_set_cursor(w->pointer, serial, w->cursor_surface, image->hotspot_x, image->hotspot_y);
    wl_surface_attach(w->cursor_surface, buffer, 0, 0);
    wl_surface_damage(w->cursor_surface, 0, 0, image->width, image->height);
    wl_surface_commit(w->cursor_surface);
}

static void pointer_handle_leave(void */*data*/, struct wl_pointer */*pointer*/, uint32_t /*serial*/, struct wl_surface */*surface*/) { }

static void pointer_handle_motion(void *data, struct wl_pointer */*pointer*/, uint32_t /*time*/, wl_fixed_t sx, wl_fixed_t sy) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    w->lastx=wl_fixed_to_int(sx);
    w->lasty=wl_fixed_to_int(sy);
    if(w->pressed) {
        pangolin::process::MouseMotion(w->lastx, w->lasty);
    }
    else {
        pangolin::process::PassiveMouseMotion(w->lastx, w->lasty);
    }
}

static void pointer_handle_button(void *data, struct wl_pointer */*wl_pointer*/, uint32_t /*serial*/, uint32_t /*time*/, uint32_t button, uint32_t state) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    if(!wl_button_ids.count(button))
        return;

    const int button_id = wl_button_ids.at(button);
    w->pressed = (state==WL_POINTER_BUTTON_STATE_PRESSED);
    pangolin::process::Mouse(button_id, (state==WL_POINTER_BUTTON_STATE_RELEASED), w->lastx, w->lasty);
}

static void pointer_handle_axis(void *data, struct wl_pointer */*wl_pointer*/, uint32_t /*time*/, uint32_t axis, wl_fixed_t value) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    int button_id = -1;
    switch (axis) {
    case REL_X: button_id = (value<0) ? 3 : 4; break;   // up, down
    case REL_Y: button_id = (value<0) ? 5 : 6; break;   // left, right
    }

    if(button_id>0) {
        pangolin::process::Mouse(button_id, 0, w->lastx, w->lasty);
    }
}

static void pointer_handle_frame(void */*data*/, struct wl_pointer */*wl_pointer*/) { }

static void pointer_handle_axis_source(void */*data*/, struct wl_pointer */*wl_pointer*/, uint32_t /*axis_source*/) { }

static void pointer_handle_axis_stop(void */*data*/, struct wl_pointer */*wl_pointer*/, uint32_t /*time*/, uint32_t /*axis*/) { }

static void pointer_handle_axis_discrete(void */*data*/, struct wl_pointer */*wl_pointer*/, uint32_t /*axis*/, int32_t /*discrete*/) { }

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

static void keyboard_handle_keymap(void *data, struct wl_keyboard */*keyboard*/, uint32_t /*format*/, int fd, uint32_t size) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    char *keymap_string = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
    xkb_keymap_unref(w->keymap);
    w->keymap = xkb_keymap_new_from_string(w->xkb_context, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(keymap_string, size);
    close(fd);
    xkb_state_unref(w->xkb_state);
    w->xkb_state = xkb_state_new(w->keymap);
}

static void keyboard_handle_enter(void */*data*/, struct wl_keyboard */*keyboard*/, uint32_t /*serial*/, struct wl_surface */*surface*/, struct wl_array */*keys*/) { }

static void keyboard_handle_leave(void */*data*/, struct wl_keyboard */*keyboard*/, uint32_t /*serial*/, struct wl_surface */*surface*/) { }

static void keyboard_handle_key(void *data, struct wl_keyboard */*keyboard*/, uint32_t /*serial*/, uint32_t /*time*/, uint32_t key, uint32_t state) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    // modifier keys
    if(wl_key_mod_ids.count(key)) {
        if(state==WL_KEYBOARD_KEY_STATE_PRESSED) {
            pangolin::context->mouse_state |=  wl_key_mod_ids.at(key);
        }
        else if (state==WL_KEYBOARD_KEY_STATE_RELEASED) {
            pangolin::context->mouse_state &= ~wl_key_mod_ids.at(key);
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
        const uint32_t utf32 = xkb_state_key_get_utf32(w->xkb_state, key+8);
        // filter non-ASCII
        if(utf32>0 && utf32<=127) {
            pango_key = int(utf32);
        }
    }

    if(pango_key>0) {
        if(state==WL_KEYBOARD_KEY_STATE_PRESSED) {
            pangolin::process::Keyboard(uint8_t(pango_key), w->lastx, w->lasty);
        }else if(state==WL_KEYBOARD_KEY_STATE_RELEASED){
            pangolin::process::KeyboardUp(uint8_t(pango_key), w->lastx, w->lasty);
        }
    }
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard */*keyboard*/, uint32_t /*serial*/, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    xkb_state_update_mask(w->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

static void keyboard_handle_repeat_info(void */*data*/, struct wl_keyboard */*wl_keyboard*/, int32_t /*rate*/, int32_t /*delay*/) { }

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
        w->wcompositor = reinterpret_cast<wl_compositor*> (wl_registry_bind(registry, id, &wl_compositor_interface, version));
    } else if (strcmp(interface, wl_shell_interface.name) == 0) {
        w->wshell = reinterpret_cast<wl_shell*> (wl_registry_bind(registry, id, &wl_shell_interface, version));
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        w->wseat = reinterpret_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, version));
        wl_seat_add_listener(w->wseat, &seat_listener, data);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        w->shm = static_cast<wl_shm*>(wl_registry_bind(registry, id, &wl_shm_interface, version));
        w->cursor_theme = wl_cursor_theme_load(nullptr, 16, w->shm);
        w->default_cursor = wl_cursor_theme_get_cursor(w->cursor_theme, "left_ptr");
    }
}

static void global_registry_remover(void */*data*/, struct wl_registry */*registry*/, uint32_t /*id*/) { }

static const struct wl_registry_listener wregistry_listener = {
    global_registry_handler,
    global_registry_remover
};

static void handle_ping(void */*data*/, struct wl_shell_surface *shell_surface, uint32_t serial) {
    wl_shell_surface_pong(shell_surface, serial);
}

static void handle_configure(void *data, struct wl_shell_surface */*shell_surface*/, uint32_t /*edges*/, int32_t width, int32_t height) {
    WaylandDisplay* const w = static_cast<WaylandDisplay*>(data);

    // notify EGL about resized area
    wl_egl_window_resize(w->egl_window, width, height, 0, 0);

    // set opaque region
    struct wl_region* wregion = wl_compositor_create_region(w->wcompositor);
    wl_region_add(wregion, 0, 0, width, height);
    wl_surface_set_opaque_region(w->wsurface, wregion);
    wl_region_destroy(wregion);

    // notify Panglin views about resized area
    pangolin::process::Resize(width, height);
}

static void handle_popup_done(void */*data*/, struct wl_shell_surface */*shell_surface*/) { }

static const struct wl_shell_surface_listener shell_surface_listener = {
    handle_ping,
    handle_configure,
    handle_popup_done
};

WaylandDisplay::WaylandDisplay(const int width, const int height, const std::string title) : width(width), height(height) {
    xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    wdisplay = wl_display_connect(nullptr);
    if (wdisplay == nullptr) { return; }

    wregistry = wl_display_get_registry(wdisplay);
    wl_registry_add_listener(wregistry, &wregistry_listener, this);

    wl_display_roundtrip(wdisplay);

    egl_display = eglGetDisplay(wdisplay);
    if(!egl_display) {
        std::cerr << "Failed to open Wayland display" << std::endl;
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

    wsurface = wl_compositor_create_surface(wcompositor);

    // set opaque background
    struct wl_region* wregion = wl_compositor_create_region(wcompositor);
    wl_region_add(wregion, 0, 0, width, height);
    wl_surface_set_opaque_region(wsurface, wregion);
    wl_region_destroy(wregion);

    egl_window = wl_egl_window_create(wsurface, width, height);
    if(!egl_window) {
        std::cerr << "Cannot create EGL window" << std::endl;
    }

    egl_surface = eglCreateWindowSurface(egl_display, egl_configs[0], (EGLNativeWindowType)egl_window, nullptr);
    if (egl_surface == EGL_NO_SURFACE) {
        std::cerr << "Cannot create EGL surface" << std::endl;
    }

    wshell_surface = wl_shell_get_shell_surface(wshell, wsurface);
    wl_shell_surface_add_listener(wshell_surface, &shell_surface_listener, this);

    wl_shell_surface_set_toplevel(wshell_surface);
    wl_shell_surface_set_title(wshell_surface, title.c_str());
    wl_shell_surface_set_class(wshell_surface, title.c_str());

    wl_display_sync(wdisplay);
}

WaylandDisplay::~WaylandDisplay() {
    // cleanup EGL
    if(egl_context) eglDestroyContext(egl_display, egl_context);
    if(egl_surface) eglDestroySurface(egl_display, egl_surface);
    if(egl_window)  wl_egl_window_destroy(egl_window);
    if(egl_display) eglTerminate(egl_display);

    // cleanup Wayland
    if(wshell_surface)  wl_shell_surface_destroy(wshell_surface);
    if(wsurface)        wl_surface_destroy(wsurface);
    if(wregistry)       wl_registry_destroy(wregistry);
    if(wdisplay)        wl_display_disconnect(wdisplay);
}

WaylandWindow::WaylandWindow(const int w, const int h, std::unique_ptr<WaylandDisplay> display) : display(std::move(display)){
    windowed_size[0] = w;
    windowed_size[1] = h;
}

WaylandWindow::~WaylandWindow() { }

void WaylandWindow::MakeCurrent() {
    eglMakeCurrent(display->egl_display, display->egl_surface, display->egl_surface, display->egl_context);
    context = this;
}

void WaylandWindow::ToggleFullscreen() {
    if(!is_fullscreen) {
        wl_shell_surface_set_fullscreen(display->wshell_surface, WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT, 0, nullptr);
    }
    else {
        wl_shell_surface_set_toplevel(display->wshell_surface);
        wl_egl_window_resize(display->egl_window, windowed_size[0], windowed_size[1], 0, 0);
        pangolin::process::Resize(windowed_size[0], windowed_size[1]);
    }
    is_fullscreen = !is_fullscreen;

    wl_display_sync(display->wdisplay);
}

void WaylandWindow::Move(const int /*x*/, const int /*y*/) { }

void WaylandWindow::Resize(const unsigned int /*w*/, const unsigned int /*h*/) { }

void WaylandWindow::ProcessEvents() { }

void WaylandWindow::SwapBuffers() {
    eglSwapBuffers(display->egl_display, display->egl_surface);
    wl_display_roundtrip(display->wdisplay);
}


std::unique_ptr<WindowInterface> CreateWaylandWindowAndBind(const std::string window_title, const int w, const int h, const std::string /*display_name*/, const bool /*double_buffered*/, const int /*sample_buffers*/, const int /*samples*/) {

    std::unique_ptr<WaylandDisplay> newdisplay = std::unique_ptr<WaylandDisplay>(new WaylandDisplay(w, h, window_title));
    // return null pointer for fallback to X11
    if(!newdisplay->wdisplay) { return nullptr; }

    // glewInit() fails with SIGSEGV for glew < 2.0 since it links to GLX
    if(atoi((char*)glewGetString(GLEW_VERSION_MAJOR))<2)
        return nullptr;

    WaylandWindow* win = new WaylandWindow(w, h, std::move(newdisplay));

    return std::unique_ptr<WindowInterface>(win);
}

} // namespace wayland

PANGOLIN_REGISTER_FACTORY(WaylandWindow)
{
  struct WaylandWindowFactory : public FactoryInterface<WindowInterface> {
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

    auto factory = std::make_shared<WaylandWindowFactory>();
    FactoryRegistry<WindowInterface>::I().RegisterFactory(factory, 9, "linux");
}

} // namespace pangolin

