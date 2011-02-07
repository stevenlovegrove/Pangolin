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

struct DataSequence
{
  DataSequence(unsigned int buffer_size = 1024);

  void Add(float val);
  void Clear();

  int x_offset;
  boost::circular_buffer<float> y;
  unsigned n;
  float sum_y;
  float sum_y_sq;
  float min_y;
  float max_y;
};

struct DataLog
{
  DataLog(unsigned int buffer_size = 1024);
  void Log(float v);
  void Log(float v1, float v2);
  void Log(float v1, float v2, float v3);
  void Log(float v1, float v2, float v3, float v4);
  void Log(float v1, float v2, float v3, float v4, float v5);
  void Log(float v1, float v2, float v3, float v4, float v5, float v6);
  void Log(unsigned int N, const float vals[]);
  void Clear();

  unsigned int buffer_size;
  int x;
  std::vector<DataSequence> sequences;
};

struct Plotter : public View, Handler
{
  Plotter(DataLog& log);
  void Render();

  DataLog* log;
//  std::vector<std::pair<int,DataLog*> > data;
};

View& CreatePlotter(const std::string& name, DataLog& log);

} // namespace pangolin

#endif // PLOTTER_H
