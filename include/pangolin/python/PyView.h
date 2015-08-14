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
class PythonInterpreter;

class PythonView : public pangolin::View, pangolin::Handler
{
public:
    PythonView();

    ~PythonView();

    void Render();

    void Keyboard(View&, unsigned char key, int x, int y, bool pressed);

    void AddLine(const std::string& str);

private:
    PythonInterpreter* python;

    GlFont& font;
    GlText prompt;
    GlText current_line;
    std::deque<GlText> line_buffer;
};

}
