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

#ifndef PLOTTER_H
#define PLOTTER_H

#include "pangolin.h"
#include <vector>
#include <boost/circular_buffer.hpp>

namespace pangolin
{

struct Plotter;
struct DataLog;

Plotter& CreatePlotter(const std::string& name, DataLog* log = 0);

struct DataUnavailableException : std::exception
{
    DataUnavailableException(std::string str) : desc(str) {}
    DataUnavailableException(std::string str, std::string detail) {
        desc = str + "\n\t" + detail;
    }
    ~DataUnavailableException() throw() {}
    const char* what() const throw() { return desc.c_str(); }
    std::string desc;
};

struct DataSequence
{
  DataSequence(unsigned int buffer_size = 1024, unsigned size = 0, float val = 0.0f );

  void Add(float val);
  void Clear();
  float operator[](unsigned int i) const;

  inline bool HasData(int i) const;

  boost::circular_buffer<float> y;
  int firstn;
  int n;
  float sum_y;
  float sum_y_sq;
  float min_y;
  float max_y;
};

inline bool DataSequence::HasData(int i) const {
    return i >= (int)firstn && i < (int)n;
}

struct DataLog
{
  DataLog(unsigned int buffer_size = 10000 );
  void Log(float v);
  void Log(float v1, float v2);
  void Log(float v1, float v2, float v3);
  void Log(float v1, float v2, float v3, float v4);
  void Log(float v1, float v2, float v3, float v4, float v5);
  void Log(float v1, float v2, float v3, float v4, float v5, float v6);
  void Log(unsigned int N, const float * vals);
  void Log(const std::vector<float> & vals);
  void SetLabels(const std::vector<std::string> & labels);
  void Clear();
  void Save(std::string filename);

  unsigned int buffer_size;
  int x;
  std::vector<DataSequence> sequences;
  std::vector<std::string> labels;
};

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


const static float colour_bg[3] = {0.0,0.0,0.0};
const static float colour_tk[3] = {0.1,0.1,0.1};
const static float colour_ms[3] = {0.3,0.3,0.3};
const static float colour_ax[3] = {0.5,0.5,0.5};

const static int draw_modes_n = 2;
const static int draw_modes[] = {GL_LINE_STRIP, GL_POINTS};

struct Plotter : public View, Handler
{
  Plotter(DataLog* log, float left=0, float right=600, float bottom=-1, float top=1, float tickx=30, float ticky=0.5 );
  void Render();
  void DrawSequence(const DataSequence& seq);
  void DrawSequence(const DataSequence& x,const DataSequence& y);
  void DrawSequenceHistogram(const std::vector<DataSequence>& seq);
  void DrawTicks();

  void ResetView();

  void Keyboard(View&, unsigned char key, int x, int y, bool pressed);
  void Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state);
  void MouseMotion(View&, int x, int y, int mouse_state);

  void ScreenToPlot(int x, int y);

  DataLog* log;
  bool track_front;
  float int_x_dflt[2];
  float int_y_dflt[2];
  float int_x[2];
  float int_y[2];
  float ticks[2];
  int last_mouse_pos[2];
  int mouse_state;
  float mouse_xy[2];

  int draw_mode;

  enum PLOT_MODES { TIME_SERIES, XY, STACKED_HISTOGRAM};
  static const unsigned modes_n = 3;
  unsigned plot_mode;
  const static unsigned int show_n = 9;
  bool show[show_n];
};

} // namespace pangolin

#endif // PLOTTER_H
