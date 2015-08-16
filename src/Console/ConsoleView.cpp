#include <pangolin/console/ConsoleView.h>

namespace pangolin
{

void glColour(const Colour& c)
{
    glColor4f(c.r,c.g,c.b,c.a);
}

void DrawLine(ConsoleView::Line& l)
{
    glColour(l.colour);
    l.text.Draw();
}

ConsoleView::ConsoleView(ConsoleInterpreter* interpreter)
    : interpreter(interpreter),
      font(GlFont::I())
{
    SetHandler(this);
    AddLine("Pangolin Python Command Prompt:");
    AddLine("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}

ConsoleView::~ConsoleView() {
    delete interpreter;
}

void ConsoleView::ProcessOutputLines()
{
    // empty output queue
    ConsoleLine line_in;
    while(interpreter->PullLine(line_in))
    {
        line_buffer.push_front(
            font.Text("%s", line_in.text.c_str())
        );
    }
}


void ConsoleView::Render()
{
    ProcessOutputLines();

    this->ActivatePixelOrthographic();
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TRANSFORM_BIT );

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA );
    glDisable(GL_DEPTH_TEST );

    glColor4f( 1.0, 0.5, 0.5, 0.8 );

    GLfloat verts[] = { 0.0f, (GLfloat)v.h,
                        (GLfloat)v.w, (GLfloat)v.h,
                        (GLfloat)v.w, 0.0f,
                        0.0f, 0.0f };
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);


    const int line_space = 15;
    glTranslated(10.0, 10.0, 0.0 );
    DrawLine(current_line);
    glTranslated(0.0, line_space, 0.0);
    for(int l=0; l < line_buffer.size(); ++l) {
        DrawLine(line_buffer[l]);
        glTranslated(0.0, line_space, 0.0);
    }

    glPopAttrib();
}

void ConsoleView::Keyboard(View&, unsigned char key, int x, int y, bool pressed)
{
    if(pressed) {
        if(key=='\r') key = '\n';

        GlText& txt = current_line.text;
        const std::string cmd = txt.Text();

        if(key=='\n') {
            interpreter->PushCommand(cmd);
            line_buffer.push_front(current_line);
            txt.Clear();
        }else if(key=='\t') {
            std::vector<std::string> options = interpreter->Complete(cmd,100);
            if(options.size()) {
                const std::string option = options[0];
                current_line = font.Text("%s", option.c_str());
            }
        }else if(key=='\b') {
            txt = font.Text("%s", txt.Text().substr(0,txt.Text().size()-1).c_str() );
        }else{
            txt = font.Text("%s%c", txt.Text().c_str(), key);
        }
    }
}

void ConsoleView::AddLine(const std::string& text, ConsoleLineType linetype, Colour colour )
{
    line_buffer.push_front( Line( font.Text("%s",text.c_str()), linetype, colour) );
}

}
