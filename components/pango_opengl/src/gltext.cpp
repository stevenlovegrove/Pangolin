/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
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

#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/gltext.h>

namespace pangolin
{

// Here just temporarily
const GLuint DEFAULT_LOCATION_POSITION = 0;
const GLuint DEFAULT_LOCATION_COLOUR = 1;
const GLuint DEFAULT_LOCATION_NORMAL = 2;
const GLuint DEFAULT_LOCATION_TEXCOORD = 3;

const char DEFAULT_NAME_POSITION[] = "a_position";
const char DEFAULT_NAME_COLOUR[] = "a_color";
const char DEFAULT_NAME_NORMAL[] = "a_normal";
const char DEFAULT_NAME_TEXCOORD[] = "a_texcoord";

GlText::GlText() :
    tex(NULL),
    width(0),
    ymin(std::numeric_limits<GLfloat>::max()),
    ymax(-std::numeric_limits<GLfloat>::max())
{
}

GlText::GlText(const GlText& txt) :
    tex(txt.tex),
    str(txt.str),
    width(txt.width),
    ymin(txt.ymin),
    ymax(txt.ymax),
    vs(txt.vs)
{
}

GlText::GlText(const GlTexture& font_tex) :
    tex(&font_tex),
    width(0),
    ymin(std::numeric_limits<GLfloat>::max()),
    ymax(-std::numeric_limits<GLfloat>::max())
{
}

void GlText::AddSpace(GLfloat s) { width += s; }

void GlText::Add(const GlChar& glc)
{
  GLfloat x = width;

  vs.push_back(glc.GetVert(0) + x);
  vs.push_back(glc.GetVert(1) + x);
  vs.push_back(glc.GetVert(2) + x);
  vs.push_back(glc.GetVert(0) + x);
  vs.push_back(glc.GetVert(2) + x);
  vs.push_back(glc.GetVert(3) + x);

  ymin = std::min(ymin, glc.YMin());
  ymax = std::max(ymax, glc.YMax());
  width = x + glc.StepX();
}

void GlText::Clear()
{
  str.clear();
  vs.clear();
  width = 0;
  ymin = +std::numeric_limits<GLfloat>::max();
  ymax = -std::numeric_limits<GLfloat>::max();
}

void GlText::DrawGlSl() const
{
#if !defined(HAVE_GLES) || defined(HAVE_GLES_2)
  if (vs.size() && tex) {
    glEnableVertexAttribArray(pangolin::DEFAULT_LOCATION_POSITION);
    glEnableVertexAttribArray(pangolin::DEFAULT_LOCATION_TEXCOORD);

    glVertexAttribPointer(
        pangolin::DEFAULT_LOCATION_POSITION, 2, GL_FLOAT, GL_FALSE,
        sizeof(XYUV), &vs[0].x);
    glVertexAttribPointer(
        pangolin::DEFAULT_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
        sizeof(XYUV), &vs[0].tu);

    tex->Bind();
    glEnable(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vs.size());
    glDisable(GL_TEXTURE_2D);

    glDisableVertexAttribArray(pangolin::DEFAULT_LOCATION_POSITION);
    glDisableVertexAttribArray(pangolin::DEFAULT_LOCATION_TEXCOORD);
  }
#endif
}

void SetWindowOrthographicContinuous() { PANGO_UNIMPLEMENTED(); }

void GlText::Draw() const
{
  if (vs.size() && tex) {
    glVertexPointer(2, GL_FLOAT, sizeof(XYUV), &vs[0].x);
    glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(XYUV), &vs[0].tu);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    tex->Bind();
    glEnable(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vs.size());
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }
}

void GlText::Draw(GLfloat x, GLfloat y, GLfloat z) const
{
  // find object point (x,y,z)' in pixel coords
  GLdouble projection[16];
  GLdouble modelview[16];
  GLint view[4];
  GLdouble scrn[3];

#ifdef HAVE_GLES_2
  std::copy(
      glEngine().projection.top().m, glEngine().projection.top().m + 16,
      projection);
  std::copy(
      glEngine().modelview.top().m, glEngine().modelview.top().m + 16,
      modelview);
#else
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
#endif
  glGetIntegerv(GL_VIEWPORT, view);

  pangolin::glProject(
      x, y, z, modelview, projection, view, scrn, scrn + 1, scrn + 2);

  // Save current state
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  SetWindowOrthographicContinuous();
  glTranslatef(
      std::floor((GLfloat)scrn[0]), std::floor((GLfloat)scrn[1]),
      (GLfloat)scrn[2]);
  Draw();

  // Restore viewport & matrices
  glViewport(view[0], view[1], view[2], view[3]);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

// Render at (x,y) in window coordinates.
void GlText::DrawWindow(GLfloat x, GLfloat y, GLfloat z) const
{
  // Backup viewport & matrices
  GLint view[4];
  glGetIntegerv(GL_VIEWPORT, view);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  SetWindowOrthographicContinuous();
  glTranslatef(std::floor(x), std::floor(y), z);
  Draw();

  // Restore viewport & matrices
  glViewport(view[0], view[1], view[2], view[3]);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

}  // namespace pangolin
