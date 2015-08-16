#pragma once

#include <deque>

#include <pangolin/platform.h>
#include <pangolin/gl/glfont.h>
#include <pangolin/var/var.h>
#include <pangolin/display/view.h>
#include <pangolin/handler/handler.h>
#include <pangolin/gl/colour.h>

#include <pangolin/Console/ConsoleInterpreter.h>

namespace pangolin
{

class ConsoleView : public pangolin::View, pangolin::Handler
{
public:
    struct Line
    {
        Line()
        {
        }

        Line(const GlText& text, ConsoleLineType linetype = ConsoleLineTypeCmd, Colour colour = Colour(1.0,1.0,1.0) )
            : text(text), linetype(linetype), colour(colour)
        {
        }

        GlText text;
        ConsoleLineType linetype;
        Colour colour;
    };


    // Construct with interpreter (and take ownership)
    ConsoleView(ConsoleInterpreter* interpreter);

    ~ConsoleView();

    void Render();

    void Keyboard(View&, unsigned char key, int x, int y, bool pressed) PANGOLIN_OVERRIDE;

private:
    void ProcessOutputLines();

    void AddLine(const std::string& text, ConsoleLineType linetype = ConsoleLineTypeCmd, Colour colour = Colour(1.0,1.0,1.0) );

    ConsoleInterpreter* interpreter;

    GlFont& font;

    std::map<ConsoleLineType, GlText> prompts;

    Line current_line;
    std::deque<Line> line_buffer;
};

}
