#include <pangolin/console/ConsoleView.h>
#include <iterator>

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

    line_colours[ConsoleLineTypeCmd]        = Colour(1.0,1.0,1.0,1.0);
    line_colours[ConsoleLineTypeCmdOptions] = Colour(0.9,0.9,0.9,1.0);
    line_colours[ConsoleLineTypeOutput]     = Colour(0.0,1.0,1.0,1.0);
    line_colours[ConsoleLineTypeHelp]       = Colour(1.0,0.8,1.0,1.0);

    line_colours[ConsoleLineTypeStdout]     = Colour(0.0,0.0,1.0,1.0);
    line_colours[ConsoleLineTypeStderr]     = Colour(1.0,0.8,0.8,1.0);

    AddLine("Pangolin Python Command Prompt:", ConsoleLineTypeHelp);
    AddLine("===============================", ConsoleLineTypeHelp);

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
        AddLine(line_in.text, line_in.linetype);
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

    glColor4f( 0.8, 0.3, 0.3, 0.8 );

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
    for(size_t l=0; l < line_buffer.size(); ++l) {
        DrawLine(line_buffer[l]);
        glTranslated(0.0, line_space, 0.0);
    }

    glPopAttrib();
}

inline std::string CommonPrefix(const std::vector<std::string>& vec)
{
    if(!vec.size()) return "";

    size_t cmn = vec[0].size();
    for(size_t i=1; i<vec.size(); ++i) {
        cmn = std::min(vec[i].size(), cmn);
        for(size_t p=0; p < cmn; ++p) {
            if(vec[i][p] != vec[0][p]) {
                cmn = p;
                break;
            }
        }
    }

    return vec[0].substr(0,cmn);
}

void ConsoleView::Keyboard(View&, unsigned char key, int x, int y, bool pressed)
{
    static int hist_id = -1;
    static std::string prefix;
    static bool edited = true;

    if(pressed) {
        if(key=='\r') key = '\n';

        GlText& txt = current_line.text;
        const std::string cmd = txt.Text();

        if(key=='\n') {
            interpreter->PushCommand(cmd);
            line_buffer.push_front(current_line);
            hist_id = -1;
            prefix = "";
            edited = true;
            txt.Clear();
        }else if(key=='\t') {
            std::vector<std::string> options = interpreter->Complete(cmd,100);
            if(options.size()) {
                const std::string common = CommonPrefix(options);
                if(common != cmd) {
                    current_line = font.Text("%s", common.c_str());
                }else{
                    std::stringstream s;
                    std::copy(options.begin(), options.end(), std::ostream_iterator<std::string>(s,", "));
                    AddLine(s.str(), ConsoleLineTypeCmdOptions);
                }
            }
        }else if(key==PANGO_SPECIAL + PANGO_KEY_UP) {
            if(edited) {
                prefix = cmd;
                edited = false;
            }
            Line* hist_line = GetLine(hist_id+1, ConsoleLineTypeCmd, prefix);
            if(hist_line) {
                current_line = *hist_line;
                hist_id++;
            }
        }else if(key==PANGO_SPECIAL + PANGO_KEY_DOWN) {
            if(edited) {
                prefix = cmd;
                edited = false;
            }
            Line* hist_line = GetLine(hist_id-1, ConsoleLineTypeCmd, prefix);
            if(hist_line) {
                current_line = *hist_line;
                hist_id--;
            }
        }else if(key=='\b') {
            txt = font.Text("%s", txt.Text().substr(0,txt.Text().size()-1).c_str() );
            edited = true;
        }else if(key < PANGO_SPECIAL){
            txt = font.Text("%s%c", txt.Text().c_str(), key);
            edited = true;
        }
    }
}

void ConsoleView::AddLine(const std::string& text, ConsoleLineType linetype )
{
    const Colour& colour = line_colours[linetype];
    line_buffer.push_front( Line( font.Text("%s",text.c_str()), linetype, colour) );
}

ConsoleView::Line* ConsoleView::GetLine(int id, ConsoleLineType line_type, const std::string& prefix )
{
    int match = 0;
    for(Line& l : line_buffer)
    {
        if(l.linetype == line_type && l.text.Text().substr(0,prefix.size()) == prefix  ) {
            if(id == match) {
                return &l;
            }else{
                ++match;
            }
        }
    }

    return nullptr;
}

}
