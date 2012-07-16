/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#include "plotter.h"
#include "display_internal.h"

#include <limits>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <boost/unordered_map.hpp>

using namespace std;

namespace pangolin
{

extern __thread PangolinGl* context;

static void* font = GLUT_BITMAP_HELVETICA_12;

DataSequence::DataSequence(unsigned int buffer_size, unsigned size, float val )
  : y(buffer_size,size,val), n(0), sum_y(0), sum_y_sq(0),
    min_y(numeric_limits<float>::max()),
    max_y(numeric_limits<float>::min())
{

}

void DataSequence::Add(float val)
{
  y.push_back(val);
  ++n;
  firstn = n-y.size();
  min_y = std::min(min_y,val);
  max_y = std::max(max_y,val);
  sum_y += val;
  sum_y_sq += val*val;
}

void DataSequence::Clear()
{
  y.clear();
  n = 0;
  firstn = 0;
  sum_y = 0;
  sum_y_sq = 0;
  min_y = numeric_limits<float>::max();
  max_y = numeric_limits<float>::min();
}

float DataSequence::operator[](unsigned int i) const
{
  if( !HasData(i) ) {
    throw DataUnavailableException("Out of range");
  }
  return y[(i-firstn) % y.size()];
}

DataLog::DataLog(unsigned int buffer_size)
  : buffer_size(buffer_size), x(0)
{
}

void DataLog::Clear()
{
  x = 0;
  sequences.clear();
}

void DataLog::Save(std::string filename)
{
  if( sequences.size() > 0 )
  {
    ofstream f(filename.c_str());
    for( int n=sequences[0].firstn; n < sequences[0].n; ++n )
    {
      f << setprecision(12) << sequences[0][n];
      for( unsigned s=1; s < sequences.size(); ++s )
        f << ", " << sequences[s][n];
      f << endl;
    }
    f.close();
  }
}

void DataLog::Log(const vector<float> & vals)
{
  Log(vals.size(), &vals[0]);
}

void DataLog::Log(unsigned int N, const float * vals)
{
  // Create new plots if needed
  for( unsigned int i= sequences.size(); i < N; ++i )
    sequences.push_back(DataSequence(buffer_size,x,0));

  // Add data to existing plots
  for( unsigned int i=0; i<sequences.size(); ++i )
    sequences[i].Add(vals[i]);

  // Fill missing data
  for( unsigned int i=N; i<sequences.size(); ++i )
    sequences[i].Add(0.0f);

  ++x;
}

void DataLog::SetLabels(const std::vector<std::string> & new_labels)
{
  // Create new labels if needed
  for( unsigned int i= labels.size(); i < new_labels.size(); ++i )
    labels.push_back(std::string("N/A"));

  // Add data to existing plots
  for( unsigned int i=0; i<labels.size(); ++i )
    labels[i] = new_labels[i];
}

void DataLog::Log(float v)
{
  const float vs[] = {v};
  Log(1,vs);
}

void DataLog::Log(float v1, float v2)
{
  const float vs[] = {v1,v2};
  Log(2,vs);
}

void DataLog::Log(float v1, float v2, float v3)
{
  const float vs[] = {v1,v2,v3};
  Log(3,vs);
}
void DataLog::Log(float v1, float v2, float v3, float v4)
{
  const float vs[] = {v1,v2,v3,v4};
  Log(4,vs);
}
void DataLog::Log(float v1, float v2, float v3, float v4, float v5)
{
  const float vs[] = {v1,v2,v3,v4,v5};
  Log(5,vs);
}
void DataLog::Log(float v1, float v2, float v3, float v4, float v5, float v6)
{
  const float vs[] = {v1,v2,v3,v4,v5,v6};
  Log(6,vs);
}

Plotter::Plotter(DataLog* log, float left, float right, float bottom, float top, float tickx, float ticky)
  : log(log), track_front(true), draw_mode(0), plot_mode(TIME_SERIES)
{
  this->handler = this;
  int_x[0] = int_x_dflt[0] = left;
  int_x[1] = int_x_dflt[1] = right;
  int_y[0] = int_y_dflt[0] = bottom;
  int_y[1] = int_y_dflt[1] = top;
  ticks[0] = tickx;
  ticks[1] = ticky;

  for( unsigned int i=0; i<show_n; ++i )
    show[i] = true;
}

void Plotter::DrawTicks()
{
  glColor3fv(colour_tk);
  const int tx[2] = {
    (int)ceil(int_x[0] / ticks[0]),
    (int)ceil(int_x[1] / ticks[0])
  };
  const int ty[2] = {
    (int)ceil(int_y[0] / ticks[1]),
    (int)ceil(int_y[1] / ticks[1])
  };

  if( tx[1] - tx[0] < v.w/4 )
  {
    for( int i=tx[0]; i<tx[1]; ++i )
    {
      glBegin(GL_LINE_STRIP);
      glVertex2f(i*ticks[0], int_y[0]);
      glVertex2f(i*ticks[0], int_y[1]);
      glEnd();
    }
  }

  if( ty[1] - ty[0] < v.h/4 )
  {
    for( int i=ty[0]; i<ty[1]; ++i )
    {
      glBegin(GL_LINE_STRIP);
      glVertex2f(int_x[0], i*ticks[1]);
      glVertex2f(int_x[1], i*ticks[1]);
      glEnd();
    }
  }

  glColor3fv(colour_ax);
  glBegin(GL_LINE_STRIP);
  glVertex2f(0, int_y[0]);
  glVertex2f(0, int_y[1]);
  glEnd();
  glBegin(GL_LINE_STRIP);
  glVertex2f(int_x[0],0);
  glVertex2f(int_x[1],0);
  glEnd();
}

void Plotter::DrawSequence(const DataSequence& seq)
{
  const int seqint_x[2] = {seq.firstn, seq.n };
  const int valid_int_x[2] = {
    std::max(seqint_x[0],(int)int_x[0]),
    std::min(seqint_x[1],(int)int_x[1])
  };
  glBegin(draw_modes[draw_mode]);
  for( int x=valid_int_x[0]; x<valid_int_x[1]; ++x )
    glVertex2f(x,seq[x]);
  glEnd();
}

void Plotter::DrawSequenceHistogram(const std::vector<DataSequence>& seq)
{
  size_t vec_size
      = std::min((unsigned)log->x, log->buffer_size);
  int idx_subtract
      = std::max(0,(int)(log->x)-(int)(log->buffer_size));
  vector<float> accum_vec(vec_size,0);

  for(int s=log->sequences.size()-1; s >=0; --s )
  {
    if( (s > 9) ||  show[s] )
    {

      const int seqint_x[2] = {seq.at(s).firstn, seq.at(s).n };
      const int valid_int_x[2] = {
        std::max(seqint_x[0],(int)int_x[0]),
        std::min(seqint_x[1],(int)int_x[1])
      };


      glBegin(GL_TRIANGLE_STRIP);
      glColor3fv(plot_colours[s%num_plot_colours]);


      for( int x=valid_int_x[0]; x<valid_int_x[1]; ++x )
      {
        float val = seq.at(s)[x];

        float & accum = accum_vec.at(x-idx_subtract);
        float before_val = accum;
        accum += val;
        glVertex2f(x-0.5,before_val);
        glVertex2f(x-0.5,accum);
        glVertex2f(x+0.5,before_val);
        glVertex2f(x+0.5,accum);
      }
      glEnd();
    }
  }
}

void Plotter::DrawSequence(const DataSequence& x,const DataSequence& y)
{
  const unsigned minn = max(x.firstn,y.firstn);
  const unsigned maxn = min(x.n,y.n);

  glBegin(draw_modes[draw_mode]);
  for( unsigned n=minn; n < maxn; ++n )
    glVertex2f(x[n],y[n]);
  glEnd();
}

void Plotter::Render()
{
  if( track_front )
  {
    const float d = int_x[1] - log->x;
    int_x[0] -= d;
    int_x[1] -= d;
  }

  ActivateScissorAndClear();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(int_x[0], int_x[1], int_y[0], int_y[1]);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_LINE_SMOOTH);

  DrawTicks();

  if( log && log->sequences.size() > 0 )
  {
    if( plot_mode==XY )
    {
      for( unsigned int s=0; s < log->sequences.size() / 2; ++s )
      {
        if( (s > 9) ||  show[s] )
        {
          glColor3fv(plot_colours[s%num_plot_colours]);
          DrawSequence(log->sequences[2*s],log->sequences[2*s+1]);
        }
      }
    }
    else if( plot_mode==TIME_SERIES)
    {
      for( unsigned int s=0; s < log->sequences.size(); ++s )
      {
        if( (s > 9) ||  show[s] )
        {
          glColor3fv(plot_colours[s%num_plot_colours]);
          DrawSequence(log->sequences[s]);
        }
      }
    }
    else if( plot_mode==STACKED_HISTOGRAM )
    {


      DrawSequenceHistogram(log->sequences);

    }
    else
    {
      assert(false);
    }
  }

  if( mouse_state & MouseButtonLeft )
  {
    if( plot_mode==XY )
    {
      glColor3fv(colour_ms);
      glBegin(GL_LINE_STRIP);
      glVertex2f(mouse_xy[0],int_y[0]);
      glVertex2f(mouse_xy[0],int_y[1]);
      glEnd();
      glBegin(GL_LINE_STRIP);
      glVertex2f(int_x[0],mouse_xy[1]);
      glVertex2f(int_x[1],mouse_xy[1]);
      glEnd();
      stringstream ss;
      ss << "(" << mouse_xy[0] << "," << mouse_xy[1] << ")";
      glColor3f(1.0,1.0,1.0);
      OpenGlRenderState::ApplyWindowCoords();
      glRasterPos2f( v.l+5,v.b+5 );
      glutBitmapString(font,(unsigned char*)ss.str().c_str());
    }else{
      int xu = (int)mouse_xy[0];
      glColor3fv(colour_ms);
      glBegin(GL_LINE_STRIP);
      glVertex2f(xu,int_y[0]);
      glVertex2f(xu,int_y[1]);
      glEnd();
      stringstream ss;
      glColor3f(1.0,1.0,1.0);
      ss << "x=" << xu << " ";
      OpenGlRenderState::ApplyWindowCoords();
      int tx = v.l+5;
      glRasterPos2f( tx,v.b+5 );
      glutBitmapString(font,(unsigned char*)ss.str().c_str());
      tx += glutBitmapLength(font,(unsigned char*)ss.str().c_str());
      for( unsigned int s=0; s<log->sequences.size(); ++s )
      {
        if( (s > show_n || show[s]) && log->sequences[s].HasData(xu) )
        {
          stringstream ss;
          ss << " " << log->sequences[s][xu];
          glColor3fv(plot_colours[s%num_plot_colours]);
          glRasterPos2f( tx,v.b+5 );
          glutBitmapString(font,(unsigned char*)ss.str().c_str());
          tx += glutBitmapLength(font,(unsigned char*)ss.str().c_str());
        }
      }
    }
  }

  float ty = v.h-15;
  for (size_t i=0; i<log->labels.size(); ++i)
  {
    glColor3fv(plot_colours[i%num_plot_colours]);

    OpenGlRenderState::ApplyWindowCoords();
    glRasterPos2f( v.l+5,ty);
    glutBitmapString(font,(unsigned char*)log->labels[i].c_str());
    ty -= 15;
  }
}

void Plotter::ResetView()
{
  track_front = true;
  int_x[0] = int_x_dflt[0];
  int_x[1] = int_x_dflt[1];
  int_y[0] = int_y_dflt[0];
  int_y[1] = int_y_dflt[1];
  for( unsigned int i=0; i<show_n; ++i )
    show[i] = true;
}

void Plotter::Keyboard(View&, unsigned char key, int x, int y, bool pressed)
{
  if( pressed )
  {
    if( key == 't' ) {
      track_front = !track_front;
    }else if( key == 'c' ) {
      log->Clear();
      cout << "Plotter: Clearing data" << endl;
    }else if( key == 's' ) {
      log->Save("./log.csv");
      cout << "Plotter: Saving to log.csv" << endl;
    }else if( key == 'm' ) {
      draw_mode = (draw_mode+1)%draw_modes_n;
    }else if( key == 'p' ) {
      plot_mode = (plot_mode+1)%modes_n;
      ResetView();
      if( plot_mode==XY ) {
        int_x[0] = int_y[0];
        int_x[1] = int_y[1];
        track_front = false;
      }
    }else if( key == 'r' ) {
      cout << "Plotter: Reset viewing range" << endl;
      ResetView();
    }else if( key == 'a' || key == ' ' ) {
      cout << "Plotter: Auto scale" << endl;
      if( plot_mode==XY && log->sequences.size() >= 2)
      {
        int_x[0] = log->sequences[0].min_y;
        int_x[1] = log->sequences[0].max_y;
        int_y[0] = log->sequences[1].min_y;
        int_y[1] = log->sequences[1].max_y;
      }else{
        float min_y = numeric_limits<float>::max();
        float max_y = numeric_limits<float>::min();
        for( unsigned int i=0; i<log->sequences.size(); ++i )
        {
          if( i>=show_n || show[i] )
          {
            min_y = std::min(min_y,log->sequences[i].min_y);
            max_y = std::max(max_y,log->sequences[i].max_y);
          }
        }
        if( min_y < max_y )
        {
          int_y[0] = min_y;
          int_y[1] = max_y;
        }
      }
    }else if( '1' <= key && key <= '9' ) {
      show[key-'1'] = !show[key-'1'];
    }
  }
}

void Plotter::ScreenToPlot(int x, int y)
{
  mouse_xy[0] = int_x[0] + (int_x[1]-int_x[0]) * (x - v.l) / (float)v.w;
  mouse_xy[1] = int_y[0] + (int_y[1]-int_y[0]) * (y - v.b) / (float)v.h;
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
    int_x[0] -= df[0];
    int_x[1] -= df[0];
    //    interval_y[0] -= df[1];
    //    interval_y[1] -= df[1];
  }else if(button_state == MouseButtonMiddle )
  {
    int_y[0] -= df[1];
    int_y[1] -= df[1];
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

Plotter& CreatePlotter(const string& name, DataLog* log)
{
  Plotter* v = new Plotter(log);
  //context->all_views[name] = v;
  bool inserted = context->named_managed_views.insert(name,v).second;
  if(!inserted) throw exception();
  context->base.views.push_back(v);
  return *v;
}

} // namespace pangolin
