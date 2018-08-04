#include <pangolin/factory/factory_registry.h>
#include <pangolin/display/display_internal.h>
#include <pangolin/display/display.h>

// GLFW header needs to be included after all other GL headers
#include <GLFW/glfw3.h>

#include <unordered_map>

#define GLFW_VERSION_GE(MAJ, MIN) GLFW_VERSION_MAJOR >= MAJ && GLFW_VERSION_MINOR >= MIN

namespace pangolin {

typedef unsigned int uint;

extern __thread PangolinGl* context;

struct GLFWWindow : public PangolinGl {
    GLFWWindow(const std::string& title, const int width, const int height, const bool offscreen = false);
    ~GLFWWindow() override;
    void ToggleFullscreen() override;
    void Move(int x, int y) override;
    void Resize(unsigned int w, unsigned int h) override;
    void MakeCurrent() override;
    void RemoveCurrent() override;
    void SwapBuffers() override;
    void ProcessEvents() override;

    GLFWwindow* window;

    int mouse_x, mouse_y;
    bool mouse_pressed = false;

    int window_pos_x;
    int window_pos_y;
};

std::unique_ptr<WindowInterface> CreateGLFWWindowAndBind(const std::string& window_title, const int w, const int h, const bool offscreen = false) {
    return std::unique_ptr<WindowInterface>(new GLFWWindow(window_title, w, h, offscreen));
}

PANGOLIN_REGISTER_FACTORY(GLFWWindow) {
struct GLFWWindowFactory : public FactoryInterface<WindowInterface> {
        std::unique_ptr<WindowInterface> Open(const Uri& uri) override {
          const std::string window_title = uri.Get<std::string>("window_title", "window");
          const int w = uri.Get<int>("w", 640);
          const int h = uri.Get<int>("h", 480);
          const bool offscreen = uri.Get<int>("offscreen", false);
          return std::unique_ptr<WindowInterface>(CreateGLFWWindowAndBind(window_title, w, h, offscreen));
    }
};

auto factory = std::make_shared<GLFWWindowFactory>();

FactoryRegistry<WindowInterface>::I().RegisterFactory(factory, 20,  "default");
}


static const std::unordered_map<uint, int> button_ids = {
    {GLFW_MOUSE_BUTTON_LEFT, 0},
    {GLFW_MOUSE_BUTTON_MIDDLE, 1},
    {GLFW_MOUSE_BUTTON_RIGHT, 2},
};

static const std::unordered_map<uint, KeyModifier> key_mod_ids = {
    {GLFW_KEY_LEFT_SHIFT, KeyModifierShift},
    {GLFW_KEY_RIGHT_SHIFT, KeyModifierShift},
    {GLFW_KEY_LEFT_CONTROL, KeyModifierCtrl},
    {GLFW_KEY_RIGHT_CONTROL, KeyModifierCtrl},
    {GLFW_KEY_LEFT_ALT, KeyModifierAlt},
    {GLFW_KEY_RIGHT_ALT, KeyModifierAlt},
    {GLFW_KEY_LEFT_SUPER, KeyModifierCmd},
    {GLFW_KEY_RIGHT_SUPER, KeyModifierCmd},
};

static const std::unordered_map<uint, int> key_special_ids = {
    {GLFW_KEY_F1, PANGO_KEY_F1},
    {GLFW_KEY_F2, PANGO_KEY_F2},
    {GLFW_KEY_F3, PANGO_KEY_F3},
    {GLFW_KEY_F4, PANGO_KEY_F4},
    {GLFW_KEY_F5, PANGO_KEY_F5},
    {GLFW_KEY_F6, PANGO_KEY_F6},
    {GLFW_KEY_F7, PANGO_KEY_F7},
    {GLFW_KEY_F8, PANGO_KEY_F8},
    {GLFW_KEY_F9, PANGO_KEY_F9},
    {GLFW_KEY_F10, PANGO_KEY_F10},
    {GLFW_KEY_F11, PANGO_KEY_F11},
    {GLFW_KEY_F12, PANGO_KEY_F12},

    {GLFW_KEY_LEFT, PANGO_KEY_LEFT},
    {GLFW_KEY_UP, PANGO_KEY_UP},
    {GLFW_KEY_RIGHT, PANGO_KEY_RIGHT},
    {GLFW_KEY_DOWN, PANGO_KEY_DOWN},

    {GLFW_KEY_PAGE_UP, PANGO_KEY_PAGE_UP},
    {GLFW_KEY_PAGE_DOWN, PANGO_KEY_PAGE_DOWN},
    {GLFW_KEY_HOME, PANGO_KEY_HOME},
    {GLFW_KEY_END, PANGO_KEY_END},
    {GLFW_KEY_INSERT, PANGO_KEY_INSERT},
};

static const std::unordered_map<uint, int> key_ord_ids = {
    {GLFW_KEY_TAB, PANGO_KEY_TAB},
    {GLFW_KEY_ESCAPE, PANGO_KEY_ESCAPE},
};

static void on_error(int error, const char* description) {
    std::cerr << "GLFW error "+std::to_string(error)+": "+std::string(description) << std::endl;
    exit(EXIT_FAILURE);
}

static void on_window_close(GLFWwindow */*window*/) {
    pangolin::Quit();
}

static void on_framebuffer_resize(GLFWwindow* window, int width, int height) {
    pangolin::process::Resize(width, height);
}

static void on_window_position(GLFWwindow* window, int xpos, int ypos) {
    GLFWWindow * const win = ((GLFWWindow * const)glfwGetWindowUserPointer(window));
    win->window_pos_x = xpos;
    win->window_pos_y = ypos;
}

static void on_focus(GLFWwindow * window, int focused) {
    GLFWWindow * const win = ((GLFWWindow * const)glfwGetWindowUserPointer(window));
    if(!focused) {
        pangolin::context->mouse_state = 0;
        win->mouse_pressed = false;
    }
}

static void on_keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GLFWWindow * const win = ((GLFWWindow * const)glfwGetWindowUserPointer(window));

    int pango_key = -1;

    if(key_mod_ids.count(key)) {
        // modifier keys
        switch (action) {
        case GLFW_PRESS:
            pangolin::context->mouse_state |= key_mod_ids.at(key);
            break;
        case GLFW_RELEASE:
            pangolin::context->mouse_state &= key_mod_ids.at(key);
            break;
        }
    }
    else if(key_special_ids.count(key)) {
        // special keys
        pango_key = PANGO_SPECIAL + key_special_ids.at(key);
    }
    else if(key_ord_ids.count(key)) {
        // ordinary keys (tab and escape)
        pango_key = key_ord_ids.at(key);
    }
    else {
        // set to ASCII character
        pango_key = key;
    }

    if(pango_key >=0) {
        switch (action) {
        case GLFW_PRESS:
            pangolin::process::Keyboard(uint8_t(pango_key), 0, 0);
            break;
        case GLFW_RELEASE:
            pangolin::process::KeyboardUp(uint8_t(pango_key), 0, 0);
            break;
        }
    }
}

static void on_mouse_position(GLFWwindow* window, double xpos, double ypos) {
    GLFWWindow * const win = ((GLFWWindow * const)glfwGetWindowUserPointer(window));
    win->mouse_x = int(xpos);
    win->mouse_y = int(ypos);

    if(win->mouse_pressed) {
        pangolin::process::MouseMotion(win->mouse_x, win->mouse_y);
    }
    else {
        pangolin::process::PassiveMouseMotion(win->mouse_x, win->mouse_y);
    }
}

static void on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
    GLFWWindow * const win = ((GLFWWindow * const)glfwGetWindowUserPointer(window));
    pangolin::process::Mouse(button_ids.at(button), (action==GLFW_RELEASE), win->mouse_x, win->mouse_y);
    win->mouse_pressed = (action==GLFW_PRESS);
}


GLFWWindow::GLFWWindow(const std::string& title, const int width, const int height, const bool offscreen) {
    windowed_size[0] = width;
    windowed_size[1] = height;
    is_fullscreen = false;

    // initialisation
    if(!glfwInit()) {
        pango_print_error("glfwInit() failed!\n");
    }

    // error callback
    glfwSetErrorCallback(on_error);

    if(offscreen) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

#ifdef __linux__
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
#endif

    // create window
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if(!window) {
        pango_print_error("glfwCreateWindow() failed!\n");
    }

    is_double_buffered = true;

    // user data for callbacks
    glfwSetWindowUserPointer(window, this);

    // register window handling callbacks
    glfwSetWindowCloseCallback(window, &on_window_close);
    glfwSetFramebufferSizeCallback(window, &on_framebuffer_resize);
    glfwSetWindowFocusCallback(window, &on_focus);
    glfwSetWindowPosCallback(window, &on_window_position);

    // register input callbacks
    glfwSetKeyCallback(window, &on_keyboard);
    glfwSetCursorPosCallback(window, &on_mouse_position);
    glfwSetMouseButtonCallback(window, &on_mouse_button);
}

GLFWWindow::~GLFWWindow() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GLFWWindow::ToggleFullscreen() {
#if GLFW_VERSION_GE(3,2)
    if(!is_fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else {
        glfwSetWindowMonitor(window, nullptr, window_pos_x, window_pos_y, windowed_size[0], windowed_size[1], GLFW_DONT_CARE);
    }
    is_fullscreen = !is_fullscreen;
#else
    std::cerr << "ToggleFullscreen() not supported for GLFW < 3.2" << std::endl;
#endif
}

void GLFWWindow::Move(int x, int y) {
    glfwSetWindowPos(window, x, y);
}

void GLFWWindow::Resize(unsigned int w, unsigned int h) {
    glfwSetWindowSize(window, w, h);
}

void GLFWWindow::MakeCurrent() {
    glfwMakeContextCurrent(window);
    context = this;
}

void GLFWWindow::RemoveCurrent() {
    glfwMakeContextCurrent(nullptr);
}

void GLFWWindow::SwapBuffers() {
    glfwSwapBuffers(window);
}

void GLFWWindow::ProcessEvents() {
    glfwPollEvents();
}

} // namespace pangolin
