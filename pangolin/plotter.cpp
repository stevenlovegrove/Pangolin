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

namespace pangolin
{

extern __thread PangolinGl* context;

DataSequence::DataSequence(unsigned int buffer_size)
  : y(buffer_size), n(0), sum_y(0), sum_y_sq(0),
    min_y(std::numeric_limits<float>::max()),
    max_y(std::numeric_limits<float>::min())
{

}

void DataSequence::Add(float val)
{
  y.push_back(val);
  ++n;
}

void DataSequence::Clear()
{
  y.clear();
  n = 0;
  sum_y = 0;
  sum_y_sq = 0;
  min_y = std::numeric_limits<float>::max();
  max_y = std::numeric_limits<float>::min();
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

void DataLog::Log(unsigned int N, const float vals[])
{
  for( unsigned int i= sequences.size(); i < N; ++i )
  {
    sequences.push_back(DataSequence(buffer_size));
    sequences[i].x_offset = x;
  }

  for( unsigned int i=0; i<sequences.size(); ++i )
    sequences[i].Add(vals[i]);
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

Plotter::Plotter(DataLog& log)
  : log(&log)
{
  this->handler = this;
}

void Plotter::Render()
{
  OpenGlRenderState::ApplyIdentity();
  glBegin(GL_LINE_STRIP);
  glVertex2f(-1,-1);
  glVertex2f(1,1);
  glEnd();
}

View& CreatePlotter(const std::string& name, DataLog& log)
{
    Plotter* v = new Plotter(log);
    context->all_views[name] = v;
    context->base.views.push_back(v);
    return *v;
}

} // namespace pangolin
