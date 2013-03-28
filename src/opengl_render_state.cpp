/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove, Richard Newcombe
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

#include <pangolin/opengl_render_state.h>

namespace pangolin
{

OpenGlMatrix OpenGlMatrix::Translate(double x, double y, double z)
{
    OpenGlMatrix mat;
    mat.SetIdentity();
    mat.m[12] = x;
    mat.m[13] = y;
    mat.m[14] = z;
    return mat;
}

OpenGlMatrix OpenGlMatrix::Scale(double x, double y, double z)
{
    OpenGlMatrix mat;
    mat.SetIdentity();
    mat.m[0] = x;
    mat.m[5] = y;
    mat.m[10] = z;
    return mat;
}

void OpenGlMatrix::Load() const
{
  glLoadMatrixd(m);
}

void OpenGlMatrix::Multiply() const
{
  glMultMatrixd(m);
}

void OpenGlMatrix::SetIdentity()
{
    m[0] = 1.0f;  m[1] = 0.0f;  m[2] = 0.0f;  m[3] = 0.0f;
    m[4] = 0.0f;  m[5] = 1.0f;  m[6] = 0.0f;  m[7] = 0.0f;
    m[8] = 0.0f;  m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
   m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}

OpenGlMatrix OpenGlMatrix::Transpose() const
{
    OpenGlMatrix trans;
    trans.m[0] = m[0];  trans.m[4] = m[1];  trans.m[8]  = m[2];  trans.m[12] = m[3];
    trans.m[1] = m[4];  trans.m[5] = m[5];  trans.m[9]  = m[6];  trans.m[13] = m[7];
    trans.m[2] = m[8];  trans.m[6] = m[9];  trans.m[10] = m[10]; trans.m[14] = m[11];
    trans.m[3] = m[12]; trans.m[7] = m[13]; trans.m[11] = m[14]; trans.m[15] = m[15];
    return trans;
}

OpenGlMatrix OpenGlMatrix::Inverse() const
{
    OpenGlMatrix inv;
    inv.m[0] = m[0]; inv.m[4] = m[1]; inv.m[8]  = m[2];  inv.m[12] = -(m[0]*m[12] + m[1]*m[13] + m[2]*m[14]);
    inv.m[1] = m[4]; inv.m[5] = m[5]; inv.m[9]  = m[6];  inv.m[13] = -(m[4]*m[12] + m[5]*m[13] + m[6]*m[14]);
    inv.m[2] = m[8]; inv.m[6] = m[9]; inv.m[10] = m[10]; inv.m[14] = -(m[8]*m[12] + m[9]*m[13] + m[10]*m[14]);
    inv.m[3] =    0; inv.m[7] =    0; inv.m[11] =    0;  inv.m[15] = 1;
    return inv;
}

std::ostream& operator<<(std::ostream& os, const OpenGlMatrix& mat)
{
    for(int r=0; r< 4; ++r) {
        for(int c=0; c<4; ++c) {
            std::cout << mat.m[4*c+r] << '\t';
        }
        std::cout << std::endl;
    }
    return os;
}

void OpenGlRenderState::Apply() const
{
  // Apply any stack matrices we have
  for(std::map<OpenGlStack,OpenGlMatrix>::const_iterator i = stacks.begin(); i != stacks.end(); ++i )
  {
    glMatrixMode(i->first);
    i->second.Load();
  }

  // Leave in MODEVIEW mode
  glMatrixMode(GL_MODELVIEW);

  if(follow) {
      T_cw.Multiply();
  }
}

OpenGlRenderState::OpenGlRenderState()
    : follow(false)
{
}

OpenGlRenderState::OpenGlRenderState(const OpenGlMatrix& projection_matrix)
    : follow(false)
{
  stacks[GlProjectionStack] = projection_matrix;
  stacks[GlModelViewStack] = IdentityMatrix();
}

OpenGlRenderState::OpenGlRenderState(const OpenGlMatrix& projection_matrix, const OpenGlMatrix& modelview_matrx)
    : follow(false)
{
  stacks[GlProjectionStack] = projection_matrix;
  stacks[GlModelViewStack] = modelview_matrx;
}

void OpenGlRenderState::ApplyIdentity()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

OpenGlRenderState& OpenGlRenderState::SetProjectionMatrix(OpenGlMatrix spec)
{
    stacks[GlProjectionStack] = spec;
    return *this;
}

OpenGlRenderState& OpenGlRenderState::SetModelViewMatrix(OpenGlMatrix spec)
{
    stacks[GlModelViewStack] = spec;
    return *this;
}

OpenGlRenderState& OpenGlRenderState::Set(OpenGlMatrixSpec spec)
{
  stacks[spec.type] = spec;
  return *this;
}

OpenGlMatrix operator*(const OpenGlMatrix& lhs, const OpenGlMatrix& rhs)
{
    OpenGlMatrix ret;
    pangolin::MatMul<4,4,4,double>(ret.m, lhs.m, rhs.m);
    return ret;
}

OpenGlMatrix& OpenGlRenderState::GetProjectionMatrix()
{
    return stacks[GlProjectionStack];
}

OpenGlMatrix OpenGlRenderState::GetProjectionMatrix() const
{
    std::map<OpenGlStack,OpenGlMatrix>::const_iterator i = stacks.find(GlProjectionStack);
    if( i == stacks.end() ) {
      return IdentityMatrix();
    }else{
      return i->second;
    }
}

OpenGlMatrix& OpenGlRenderState::GetModelViewMatrix()
{
    return stacks[GlModelViewStack];
}

OpenGlMatrix OpenGlRenderState::GetModelViewMatrix() const
{
    std::map<OpenGlStack,OpenGlMatrix>::const_iterator i = stacks.find(GlModelViewStack);
    if( i == stacks.end() ) {
      return IdentityMatrix();
    }else{
      return i->second;
    }
}

OpenGlMatrix OpenGlRenderState::GetProjectionModelViewMatrix() const
{
    return GetProjectionMatrix() * GetModelViewMatrix();
}

OpenGlMatrix OpenGlRenderState::GetProjectiveTextureMatrix() const
{
    return OpenGlMatrix::Translate(0.5,0.5,0.5) * OpenGlMatrix::Scale(0.5,0.5,0.5) * GetProjectionModelViewMatrix();
}

void OpenGlRenderState::EnableProjectiveTexturing() const
{
    const pangolin::OpenGlMatrix projmattrans = GetProjectiveTextureMatrix().Transpose();
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);
    glTexGendv(GL_S, GL_EYE_PLANE, projmattrans.m);
    glTexGendv(GL_T, GL_EYE_PLANE, projmattrans.m+4);
    glTexGendv(GL_R, GL_EYE_PLANE, projmattrans.m+8);
    glTexGendv(GL_Q, GL_EYE_PLANE, projmattrans.m+12);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
}

void OpenGlRenderState::DisableProjectiveTexturing() const
{
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);
}

void OpenGlRenderState::Follow(const OpenGlMatrix& T_wc, bool follow)
{
    this->T_cw = T_wc.Inverse();

    if(follow != this->follow) {
        if(follow) {
            const OpenGlMatrix T_vc = GetModelViewMatrix() * T_wc;
            SetModelViewMatrix(T_vc);
            this->follow = true;
        }else{
            Unfollow();
        }
    }
}

void OpenGlRenderState::Unfollow()
{
    const OpenGlMatrix T_vw = GetModelViewMatrix() * T_cw;
    SetModelViewMatrix(T_vw);
    this->follow = false;
}

// Use OpenGl's default frame of reference
OpenGlMatrixSpec ProjectionMatrix(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar )
{
    return ProjectionMatrixRUB_BottomLeft(w,h,fu,fv,u0,v0,zNear,zFar);
}

OpenGlMatrixSpec ProjectionMatrixOrthographic(double l, double r, double b, double t, double n, double f )
{
  OpenGlMatrixSpec P;
  P.type = GlProjectionStack;

  P.m[0] = 2/(r-l);
  P.m[1] = 0;
  P.m[2] = 0;
  P.m[3] = 0;

  P.m[4] = 0;
  P.m[5] = 2/(t-b);
  P.m[6] = 0;
  P.m[7] = 0;

  P.m[8] = 0;
  P.m[9] = 0;
  P.m[10] = -2/(f-n);
  P.m[11] = 0;

  P.m[12] = -(r+l)/(r-l);
  P.m[13] = -(t+b)/(t-b);
  P.m[14] = -(f+n)/(f-n);
  P.m[15] = 1;

  return P;
}


// Camera Axis:
//   X - Right, Y - Up, Z - Back
// Image Origin:
//   Bottom Left
// Caution: Principal point defined with respect to image origin (0,0) at
//          top left of top-left pixel (not center, and in different frame
//          of reference to projection function image)
OpenGlMatrixSpec ProjectionMatrixRUB_BottomLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar )
{
    // http://www.songho.ca/opengl/gl_projectionmatrix.html
    const double L = +(u0) * zNear / -fu;
    const double T = +(v0) * zNear / fv;
    const double R = -(w-u0) * zNear / -fu;
    const double B = -(h-v0) * zNear / fv;

    OpenGlMatrixSpec P;
    P.type = GlProjectionStack;
    std::fill_n(P.m,4*4,0);

    P.m[0*4+0] = 2 * zNear / (R-L);
    P.m[1*4+1] = 2 * zNear / (T-B);
    P.m[2*4+2] = -(zFar +zNear) / (zFar - zNear);
    P.m[2*4+0] = (R+L)/(R-L);
    P.m[2*4+1] = (T+B)/(T-B);
    P.m[2*4+3] = -1.0;
    P.m[3*4+2] =  -(2*zFar*zNear)/(zFar-zNear);

    return P;
}

// Camera Axis:
//   X - Right, Y - Down, Z - Forward
// Image Origin:
//   Top Left
// Pricipal point specified with image origin (0,0) at top left of top-left pixel (not center)
OpenGlMatrixSpec ProjectionMatrixRDF_TopLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar )
{
    // http://www.songho.ca/opengl/gl_projectionmatrix.html
    const double L = -(u0) * zNear / fu;
    const double R = +(w-u0) * zNear / fu;
    const double T = -(v0) * zNear / fv;
    const double B = +(h-v0) * zNear / fv;

    OpenGlMatrixSpec P;
    P.type = GlProjectionStack;
    std::fill_n(P.m,4*4,0);

    P.m[0*4+0] = 2 * zNear / (R-L);
    P.m[1*4+1] = 2 * zNear / (T-B);

    P.m[2*4+0] = (R+L)/(L-R);
    P.m[2*4+1] = (T+B)/(B-T);
    P.m[2*4+2] = (zFar +zNear) / (zFar - zNear);
    P.m[2*4+3] = 1.0;

    P.m[3*4+2] =  (2*zFar*zNear)/(zNear - zFar);
    return P;
}

// Camera Axis:
//   X - Right, Y - Down, Z - Forward
// Image Origin:
//   Bottom Left
// Pricipal point specified with image origin (0,0) at top left of top-left pixel (not center)
OpenGlMatrixSpec ProjectionMatrixRDF_BottomLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar )
{
    // http://www.songho.ca/opengl/gl_projectionmatrix.html
    const double L = -(u0) * zNear / fu;
    const double R = +(w-u0) * zNear / fu;
    const double B = -(v0) * zNear / fv;
    const double T = +(h-v0) * zNear / fv;

    OpenGlMatrixSpec P;
    P.type = GlProjectionStack;
    std::fill_n(P.m,4*4,0);

    P.m[0*4+0] = 2 * zNear / (R-L);
    P.m[1*4+1] = 2 * zNear / (T-B);

    P.m[2*4+0] = (R+L)/(L-R);
    P.m[2*4+1] = (T+B)/(B-T);
    P.m[2*4+2] = (zFar +zNear) / (zFar - zNear);
    P.m[2*4+3] = 1.0;

    P.m[3*4+2] =  (2*zFar*zNear)/(zNear - zFar);
    return P;
}

OpenGlMatrix ModelViewLookAtRUB(double ex, double ey, double ez, double lx, double ly, double lz, double ux, double uy, double uz)
{
  OpenGlMatrix mat;
  GLdouble* m = mat.m;

  const double u_o[3] = {ux,uy,uz};

  GLdouble x[3], y[3];
  GLdouble z[] = {ex - lx, ey - ly, ez - lz};
  Normalise<3>(z);

  CrossProduct(x,u_o,z);
  CrossProduct(y,z,x);

  Normalise<3>(x);
  Normalise<3>(y);

#define M(row,col)  m[col*4+row]
  M(0,0) = x[0];
  M(0,1) = x[1];
  M(0,2) = x[2];
  M(1,0) = y[0];
  M(1,1) = y[1];
  M(1,2) = y[2];
  M(2,0) = z[0];
  M(2,1) = z[1];
  M(2,2) = z[2];
  M(3,0) = 0.0;
  M(3,1) = 0.0;
  M(3,2) = 0.0;
  M(0,3) = -(M(0,0)*ex + M(0,1)*ey + M(0,2)*ez);
  M(1,3) = -(M(1,0)*ex + M(1,1)*ey + M(1,2)*ez);
  M(2,3) = -(M(2,0)*ex + M(2,1)*ey + M(2,2)*ez);
  M(3,3) = 1.0;
#undef M

  return mat;
}

OpenGlMatrix ModelViewLookAtRDF(double ex, double ey, double ez, double lx, double ly, double lz, double ux, double uy, double uz)
{
  OpenGlMatrix mat;
  GLdouble* m = mat.m;

  const double u_o[3] = {ux,uy,uz};

  GLdouble x[3], y[3];
  GLdouble z[] = {lx - ex, ly - ey, lz - ez};
  Normalise<3>(z);

  CrossProduct(x,z,u_o);
  CrossProduct(y,z,x);

  Normalise<3>(x);
  Normalise<3>(y);

#define M(row,col)  m[col*4+row]
  M(0,0) = x[0];
  M(0,1) = x[1];
  M(0,2) = x[2];
  M(1,0) = y[0];
  M(1,1) = y[1];
  M(1,2) = y[2];
  M(2,0) = z[0];
  M(2,1) = z[1];
  M(2,2) = z[2];
  M(3,0) = 0.0;
  M(3,1) = 0.0;
  M(3,2) = 0.0;
  M(0,3) = -(M(0,0)*ex + M(0,1)*ey + M(0,2)*ez);
  M(1,3) = -(M(1,0)*ex + M(1,1)*ey + M(1,2)*ez);
  M(2,3) = -(M(2,0)*ex + M(2,1)*ey + M(2,2)*ez);
  M(3,3) = 1.0;
#undef M

  return mat;
}

OpenGlMatrix ModelViewLookAt(double ex, double ey, double ez, double lx, double ly, double lz, double ux, double uy, double uz)
{
  return ModelViewLookAtRUB(ex,ey,ez,lz,ly,lz,ux,uy,uz);
}

OpenGlMatrix ModelViewLookAt(double ex, double ey, double ez, double lx, double ly, double lz, AxisDirection up)
{
  const double* u = AxisDirectionVector[up];
  return ModelViewLookAtRUB(ex,ey,ez,lx,ly,lz,u[0],u[1],u[2]);
}

OpenGlMatrix IdentityMatrix()
{
  OpenGlMatrix P;
  std::fill_n(P.m,4*4,0);
  for( int i=0; i<4; ++i ) P.m[i*4+i] = 1;
  return P;
}

OpenGlMatrixSpec IdentityMatrix(OpenGlStack type)
{
  OpenGlMatrixSpec P;
  P.type = type;
  std::fill_n(P.m,4*4,0);
  for( int i=0; i<4; ++i ) P.m[i*4+i] = 1;
  return P;
}

OpenGlMatrixSpec  negIdentityMatrix(OpenGlStack type)
{
  OpenGlMatrixSpec P;
  P.type = type;
  std::fill_n(P.m,4*4,0);
  for( int i=0; i<4; ++i ) P.m[i*4+i] = -1;

  P.m[3*4+3] =1;
  return P;
}

}
