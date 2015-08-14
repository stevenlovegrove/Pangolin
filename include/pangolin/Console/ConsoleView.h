#pragma once

#include <deque>

#include <pangolin/platform.h>
#include <pangolin/gl/glfont.h>
#include <pangolin/var/var.h>
#include <pangolin/display/view.h>
#include <pangolin/handler/handler.h>

#include <pangolin/Console/ConsoleInterpreter.h>

namespace pangolin
{

class ConsoleView : public pangolin::View, pangolin::Handler
{
public:
    // Construct with interpreter (and take ownership)
    ConsoleView(ConsoleInterpreter* interpreter);

    ~ConsoleView();

    void Render();

    void Keyboard(View&, unsigned char key, int x, int y, bool pressed) PANGOLIN_OVERRIDE;

    void AddLine(const std::string& str);

private:
    void ProcessOutputLines();

    ConsoleInterpreter* interpreter;

    GlFont& font;
    GlText prompt;
    GlText current_line;
    std::deque<GlText> line_buffer;
};

}
