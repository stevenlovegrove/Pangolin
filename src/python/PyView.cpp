#include <pangolin/python/PyView.h>
#include <pangolin/python/PyInterpreter.h>

namespace pangolin
{

PyView::PyView()
    : python(new PyInterpreter()),
      font(GlFont::I()),
      prompt(font.Text("")),
      current_line(prompt)
{
    SetHandler(this);
    AddLine("Pangolin Python Command Prompt:");
    AddLine("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}

PyView::~PyView() {
    delete python;
}

void PyView::ProcessOutputLines()
{
    // empty output queue
    ConsoleLine line_in;
    while(python->PullLine(line_in))
    {
        line_buffer.push_front(
            font.Text("%s", line_in.text.c_str())
        );
    }
}


void PyView::Render()
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
    glColor4f( 1.0, 1.0, 1.0, 1.0 );
    current_line.Draw();
    glTranslated(0.0, line_space, 0.0);
    for(int l=0; l < line_buffer.size(); ++l) {
        GlText& txt = line_buffer[l];
        txt.Draw();
        glTranslated(0.0, line_space, 0.0);
    }

    glPopAttrib();
}

void PyView::Keyboard(View&, unsigned char key, int x, int y, bool pressed)
{
    if(pressed) {
        if(key=='\r') key = '\n';

        const std::string cmd = current_line.Text();

        if(key=='\n') {
            python->PushCommand(cmd);
            line_buffer.push_front(current_line);
            current_line.Clear();
        }else if(key=='\t') {
            std::vector<std::string> options = python->Complete(cmd,100);
            if(options.size()) {
                const std::string option = options[0];
                current_line = font.Text("%s", option.c_str());
            }
        }else if(key=='\b') {
            current_line = font.Text("%s", current_line.Text().substr(0,current_line.Text().size()-1).c_str() );
        }else{
            current_line = font.Text("%s%c", current_line.Text().c_str(), key);
        }
    }
}

void PyView::AddLine(const std::string& str)
{
    line_buffer.push_front( font.Text("%s",str.c_str()) );
}

}
