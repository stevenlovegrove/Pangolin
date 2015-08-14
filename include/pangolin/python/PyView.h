#pragma once

#include <deque>

#include <pangolin/platform.h>
#include <pangolin/gl/glfont.h>
#include <pangolin/var/var.h>
#include <pangolin/display/view.h>
#include <pangolin/handler/handler.h>

namespace pangolin
{

// Forward declartation
class PyInterpreter;

class PyView : public pangolin::View, pangolin::Handler
{
public:
    PyView();

    ~PyView();

    void Render();

    void Keyboard(View&, unsigned char key, int x, int y, bool pressed) PANGOLIN_OVERRIDE;

    void AddLine(const std::string& str);

private:
    void ProcessOutputLines();

    PyInterpreter* python;

    GlFont& font;
    GlText prompt;
    GlText current_line;
    std::deque<GlText> line_buffer;
};

}
