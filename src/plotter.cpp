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

#include <pangolin/plotter.h>
#include <pangolin/gldraw.h>

namespace pangolin
{

const static int num_plot_colours = 12;
const static float plot_colours[][3] =
{
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0},
    {1.0, 0.0, 1.0},
    {0.5, 0.5, 0.0},
    {0.5, 0.0, 0.0},
    {0.0, 0.5, 0.0},
    {0.0, 0.0, 0.5},
    {0.5, 0.0, 1.0},
    {0.0, 1.0, 0.5},
    {1.0, 0.0, 0.5},
    {0.0, 0.5, 1.0}
};

inline void SetColor(float colour[4], float r, float g, float b, float alpha = 1.0f)
{
    colour[0] = r;
    colour[1] = g;
    colour[2] = b;
    colour[3] = alpha;
}

std::string ReplaceChar(const std::string& str, char from, char to)
{
    std::string ret = str;
    for(size_t i=0; i<ret.length(); ++i) {
        if(ret[i] == from) ret[i] = to;
    }
    return ret;
}

std::set<int> ConvertSequences(const std::string& str, char seq_char='$', char id_char='i')
{
    std::set<int> sequences;

    for(size_t i=0; i<str.length(); ++i) {
        if(str[i] == seq_char) {
            if(i+1 < str.length() && str[i+1] == id_char) {
                sequences.insert(-1);
            }else{
                int v = 0;
                for(size_t j=i+1; std::isdigit(str[j]) && j< str.length(); ++j) {
                    v = v*10 + (str[j] - '0');
                }
                sequences.insert(v);
            }
        }
    }

    return sequences;
}

void Plotter::PlotSeries::CreatePlot(const std::string &x, const std::string &y)
{
    static const std::string vs_header =
            "uniform int u_id_offset;\n"
            "uniform vec4 u_color;\n"
            "uniform vec2 u_scale;\n"
            "uniform vec2 u_offset;\n"
            "varying vec4 v_color;\n"
            "void main() {\n";

    static const std::string vs_footer =
            "    vec2 pos = vec2(x, y);\n"
            "    gl_Position = vec4(u_scale * (pos + u_offset),0,1);\n"
            "    v_color = u_color;\n"
            "}\n";

    static const std::string fs =
            "varying vec4 v_color;\n"
            "void main() {\n"
            "  gl_FragColor = v_color;\n"
            "}\n";

    attribs.clear();

    const std::set<int> ax = ConvertSequences(x);
    const std::set<int> ay = ConvertSequences(y);
    std::set<int> as;
    as.insert(ax.begin(), ax.end());
    as.insert(ay.begin(), ay.end());
    contains_id = ( as.find(-1) != as.end() );

    std::string vs_attrib;
    for(std::set<int>::const_iterator i=as.begin(); i != as.end(); ++i) {
        std::ostringstream oss;
        oss << "s" << *i;
        const std::string name = *i >= 0 ? oss.str() : "si";
        attribs.push_back( PlotAttrib(name, *i) );
        vs_attrib += "attribute float " + name + ";\n";
    }

    prog.AddShader( GlSlVertexShader,
                    vs_attrib +
                    vs_header +
                    "float x = " + ReplaceChar(x,'$','s') + ";\n" +
                    "float y = " + ReplaceChar(y,'$','s') + ";\n" +
                    vs_footer);
    prog.AddShader( GlSlFragmentShader, fs );
    prog.Link();

    // Lookup attribute locations in compiled shader
    prog.SaveBind();
    for(size_t i=0; i<attribs.size(); ++i) {
        attribs[i].location = prog.GetAttributeHandle( attribs[i].name );
    }
    prog.Unbind();

}
Plotter::Plotter(DataLog* log, float left, float right, float bottom, float top, float tickx, float ticky, Plotter* linked)
{
    if(!log) {
        throw std::runtime_error("DataLog not specified");
    }

    // Handle our own mouse / keyboard events
    this->handler = this;
    this->log = log;

    // Default colour scheme
    SetColor(colour_bg, 0.0,0.0,0.0);
    SetColor(colour_tk, 0.2,0.2,0.2);
    SetColor(colour_ms, 0.3,0.3,0.3);
    SetColor(colour_ax, 0.5,0.5,0.5);

    // Setup view range.
    int_x[0] = int_x_dflt[0] = left;
    int_x[1] = int_x_dflt[1] = right;
    int_y[0] = int_y_dflt[0] = bottom;
    int_y[1] = int_y_dflt[1] = top;
    vo[0] = vo[1] = 0;
    ticks[0] = tickx;
    ticks[1] = ticky;
    track_front = false;
    lineThickness = 1.5;
    mouse_state = 0;

    // Create shader for drawing simple primitives
    prog_default.AddShader( GlSlVertexShader,
                         "attribute vec2 a_position;\n"
                         "uniform vec4 u_color;\n"
                         "uniform vec2 u_scale;\n"
                         "uniform vec2 u_offset;\n"
                         "varying vec4 v_color;\n"
                         "void main() {\n"
                         "    gl_Position = vec4(u_scale * (a_position + u_offset),0,1);\n"
                         "    v_color = u_color;\n"
                         "}\n"
                         );
    prog_default.AddShader( GlSlFragmentShader,
                         "varying vec4 v_color;\n"
                         "void main() {\n"
                         "  gl_FragColor = v_color;\n"
                         "}\n"
                         );
    prog_default.Link();

    // Setup default PlotSeries
    plotseries.reserve(10);
    plotseries.push_back( PlotSeries() );
    plotseries.back().CreatePlot("$i", "$0");
    plotseries.push_back( PlotSeries() );
    plotseries.back().CreatePlot("$i", "$1");
    plotseries.push_back( PlotSeries() );
    plotseries.back().CreatePlot("$i", "$2");
    plotseries.push_back( PlotSeries() );
    plotseries.back().CreatePlot("$i", "$3");
    plotseries.push_back( PlotSeries() );
    plotseries.back().CreatePlot("$i", "$4");
    plotseries.push_back( PlotSeries() );
    plotseries.back().CreatePlot("$i", "$5");
}

void Plotter::Render()
{
#ifndef HAVE_GLES
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
#endif

    if( track_front )
    {
        // TODO: Implement
    }

    glClearColor(colour_bg[0], colour_bg[1], colour_bg[2], colour_bg[3]);
    ActivateScissorAndClear();

    // Try to create smooth lines
    glDisable(GL_MULTISAMPLE);
    glLineWidth(1.5);
    glEnable(GL_LINE_SMOOTH);
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glDisable( GL_DEPTH_TEST );

    const float x = int_x[0]+vo[0];
    const float y = int_y[0]+vo[1];
    const float w = int_x[1]+vo[0] - x;
    const float h = int_y[1]+vo[1] - y;
    const float ox = -(x+w/2.0f);
    const float oy = -(y+h/2.0f);

    // Draw ticks
    prog_default.SaveBind();
    prog_default.SetUniform("u_scale",  2.0f / w, 2.0f / h);
    prog_default.SetUniform("u_offset", ox, oy);

    prog_default.SetUniform("u_color",  colour_tk[0], colour_tk[1], colour_tk[2], colour_tk[3] );
    glLineWidth(lineThickness);
    const int tx[2] = {
        (int)ceil(int_x[0] / ticks[0]),
        (int)ceil(int_x[1] / ticks[0])
    };

    const int ty[2] = {
        (int)ceil(int_y[0] / ticks[1]),
        (int)ceil(int_y[1] / ticks[1])
    };

    const int votx = ceil(vo[0]/ ticks[0]);
    const int voty = ceil(vo[1]/ ticks[1]);
    if( tx[1] - tx[0] < v.w/4 ) {
        for( int i=tx[0]; i<tx[1]; ++i ) {
            glDrawLine((i+votx)*ticks[0], int_y[0]+vo[1],   (i+votx)*ticks[0], int_y[1]+vo[1]);
        }
    }

    if( ty[1] - ty[0] < v.h/4 ) {
        for( int i=ty[0]; i<ty[1]; ++i ) {
            glDrawLine(int_x[0]+vo[0], (i+voty)*ticks[1],  int_x[1]+vo[0], (i+voty)*ticks[1]);
        }
    }

    prog_default.SetUniform("u_color",  colour_ax[0], colour_ax[1], colour_ax[2], colour_ax[3] );
    glDrawLine(0, int_y[0]+vo[1],  0, int_y[1]+vo[1] );
    glDrawLine(int_x[0]+vo[0],0,   int_x[1]+vo[0],0  );
    glLineWidth(1.0f);
    prog_default.Unbind();

    static size_t id_start = 0;
    static size_t id_size = 0;
    static float* id_array = 0;

    for(size_t i=0; i < plotseries.size(); ++i)
    {
        PlotSeries& ps = plotseries[i];
        GlSlProgram& prog = ps.prog;

        prog.SaveBind();
        prog.SetUniform("u_scale",  2.0f / w, 2.0f / h);
        prog.SetUniform("u_offset", ox, oy);
        prog.SetUniform("u_color",  plot_colours[i][0], plot_colours[i][1], plot_colours[i][2], 1.0f );

        const DataLogBlock* block = log->Blocks();
        while(block) {
            if(ps.contains_id ) {
                if(id_size < block->Samples() ) {
                    // Create index array that we can bind
                    delete[] id_array;
                    id_start = -1;
                    id_size = block->MaxSamples();
                    id_array = new float[id_size];
                }
                if(id_start != block->StartId()) {
                    for(size_t k=0; k < id_size; ++k) {
                        id_array[k] = block->StartId() + k;
                    }
                }
            }

            prog.SetUniform("u_id_offset",  (int)block->StartId() );

            // Enable appropriate attributes
            for(size_t i=0; i< ps.attribs.size(); ++i) {
                if(0 <= ps.attribs[i].plot_id && ps.attribs[i].plot_id < (int)block->Dimensions() ) {
                    glVertexAttribPointer(ps.attribs[i].location, 1, GL_FLOAT, GL_FALSE, block->Dimensions()*sizeof(float), block->DimData(ps.attribs[i].plot_id) );
                    glEnableVertexAttribArray(ps.attribs[i].location);
                }else if( ps.attribs[i].plot_id == -1 ){
                    glVertexAttribPointer(ps.attribs[i].location, 1, GL_FLOAT, GL_FALSE, 0, id_array );
                    glEnableVertexAttribArray(ps.attribs[i].location);
                }else{
                    // bad id: don't render
                    goto draw_skip;
                }
            }

            // Draw geometry
            glDrawArrays(GL_LINE_STRIP, 0, block->Samples());

        draw_skip:
            // Disable enabled attributes
            for(size_t i=0; i< ps.attribs.size(); ++i) {
                glDisableVertexAttribArray(ps.attribs[i].location);
            }

            block = block->NextBlock();
        }
        prog.Unbind();
    }

#ifndef HAVE_GLES
    glPopAttrib();
#endif

}

void Plotter::Keyboard(View&, unsigned char key, int x, int y, bool pressed)
{

}

void Plotter::ScreenToPlot(int x, int y)
{
    mouse_xy[0] = vo[0] + int_x[0] + (int_x[1]-int_x[0]) * (x - v.l) / (float)v.w;
    mouse_xy[1] = vo[1] + int_y[0] + (int_y[1]-int_y[0]) * (y - v.b) / (float)v.h;
}

void Plotter::Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state)
{
    last_mouse_pos[0] = x;
    last_mouse_pos[1] = y;
    mouse_state = button_state;

    if(button == MouseWheelUp || button == MouseWheelDown)
    {
        //const float mean = (int_y[0] + int_y[1])/2.0;
        const float scale = 1.0f + ((button == MouseWheelDown) ? 0.1 : -0.1);
        //    int_y[0] = scale*(int_y[0] - mean) + mean;
        //    int_y[1] = scale*(int_y[1] - mean) + mean;
        int_y[0] = scale*(int_y[0]) ;
        int_y[1] = scale*(int_y[1]) ;
    }

    ScreenToPlot(x,y);
}

void Plotter::MouseMotion(View&, int x, int y, int button_state)
{
    mouse_state = button_state;
    const int d[2] = {x-last_mouse_pos[0],y-last_mouse_pos[1]};
    const float is[2] = {int_x[1]-int_x[0],int_y[1]-int_y[0]};
    const float df[2] = {is[0]*d[0]/(float)v.w, is[1]*d[1]/(float)v.h};

    if( button_state == MouseButtonLeft )
    {
        track_front = false;
        //int_x[0] -= df[0];
        //int_x[1] -= df[0];
        vo[0] -= df[0];

        //    interval_y[0] -= df[1];
        //    interval_y[1] -= df[1];
    }else if(button_state == MouseButtonMiddle )
    {
        //int_y[0] -= df[1];
        //int_y[1] -= df[1];
        vo[1] -= df[1];
    }else if(button_state == MouseButtonRight )
    {
        const double c[2] = {
            track_front ? int_x[1] : (int_x[0] + int_x[1])/2.0,
            (int_y[0] + int_y[1])/2.0
        };
        const float scale[2] = {
            1.0f + (float)d[0] / (float)v.w,
            1.0f - (float)d[1] / (float)v.h,
        };
        int_x[0] = scale[0]*(int_x[0] - c[0]) + c[0];
        int_x[1] = scale[0]*(int_x[1] - c[0]) + c[0];
        int_y[0] = scale[1]*(int_y[0] - c[1]) + c[1];
        int_y[1] = scale[1]*(int_y[1] - c[1]) + c[1];
    }

    last_mouse_pos[0] = x;
    last_mouse_pos[1] = y;
}

void Plotter::Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state)
{
    mouse_state = button_state;

    if(inType == InputSpecialScroll) {
        const float d[2] = {p1,-p2};
        const float is[2] = {int_x[1]-int_x[0],int_y[1]-int_y[0]};
        const float df[2] = {is[0]*d[0]/(float)v.w, is[1]*d[1]/(float)v.h};

        vo[0] -= df[0];
        vo[1] -= df[1];
        if(df[0] > 0) {
            track_front = false;
        }
    } else if(inType == InputSpecialZoom) {
        const float scale = (1-p1);
        const double c[2] = {
            track_front ? int_x[1] : (int_x[0] + int_x[1])/2.0,
            (int_y[0] + int_y[1])/2.0
        };

        if(button_state & KeyModifierCmd) {
            int_y[0] = scale*(int_y[0] - c[1]) + c[1];
            int_y[1] = scale*(int_y[1] - c[1]) + c[1];
        }else{
            int_x[0] = scale*(int_x[0] - c[0]) + c[0];
            int_x[1] = scale*(int_x[1] - c[0]) + c[0];
        }
    }
}

}
