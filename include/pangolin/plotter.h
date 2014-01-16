/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PLOTTER_H
#define PLOTTER_H

#include <pangolin/gl.h>
#include <pangolin/view.h>
#include <pangolin/handler.h>
#include <pangolin/glsl.h>
#include <pangolin/datalog.h>

namespace pangolin
{

struct PlotSeries
{
    struct Attrib
    {
        Attrib(int location, int plot_id)
            : location(location), plot_id(plot_id)
        {
        }

        int location;
        int plot_id;
    };

    void CreatePlot(const std::string& x, const std::string& y)
    {
        // TODO: Customise shader based on x and y

        prog.AddShader( GlSlVertexShader,
                             "#extension GL_EXT_gpu_shader4 : require\n"
                             "attribute float s0;\n"
                             "attribute float s1;\n"
                             "uniform int u_id_offset;\n"
                             "uniform vec4 u_color;\n"
                             "uniform vec2 u_scale;\n"
                             "uniform vec2 u_offset;\n"
                             "varying vec4 v_color;\n"
                             "void main() {\n"
                             "    vec2 pos = vec2(u_id_offset+gl_VertexID,s0);\n"
    //                         "    vec2 pos = vec2(s0, s1);\n"
                             "    gl_Position = vec4(u_scale * (pos + u_offset),0,1);\n"
                             "    v_color = u_color;\n"
                             "}\n"
                             );
        prog.AddShader( GlSlFragmentShader,
                             "varying vec4 v_color;\n"
                             "void main() {\n"
                             "  gl_FragColor = v_color;\n"
                             "}\n"
                             );
        prog.Link();

        prog.SaveBind();
        attribs.push_back( Attrib(prog.GetAttributeHandle("s0"), 0) );
        attribs.push_back( Attrib(prog.GetAttributeHandle("s1"), 1) );
        prog.Unbind();

    }

    GlSlProgram prog;
    std::vector<Attrib> attribs;
};

class Plotter : public View, Handler
{
public:
    Plotter(DataLog* log, float left=0, float right=600, float bottom=-1, float top=1, float tickx=30, float ticky=0.5, Plotter* linked = 0 );
    void Render();
    void DrawTicks();

    void ScreenToPlot(int x, int y);
    void Keyboard(View&, unsigned char key, int x, int y, bool pressed);
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state);
    void MouseMotion(View&, int x, int y, int mouse_state);
    void Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state);

protected:
    DataLog* log;
    Plotter* linked;

    float colour_bg[4];
    float colour_tk[4];
    float colour_ms[4];
    float colour_ax[4];
    float lineThickness;

    GlSlProgram prog_default;

    std::vector<PlotSeries> plotseries;

    // Rethink these ...
    bool track_front;
    float int_x_dflt[2];
    float int_y_dflt[2];
    float int_x[2];
    float int_y[2];
    float vo[2]; //view offset
    float ticks[2];
    int last_mouse_pos[2];
    int mouse_state;
    float mouse_xy[2];

};

} // namespace pangolin

#endif // PLOTTER_H
