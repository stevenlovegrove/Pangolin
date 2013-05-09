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

#ifndef PANGOLIN_OPENGLRENDERSTATE_H
#define PANGOLIN_OPENGLRENDERSTATE_H

#include <pangolin/platform.h>
#include <pangolin/glinclude.h>
#include <pangolin/simple_math.h>

#include <map>

#ifdef HAVE_EIGEN
#include <Eigen/Eigen>
#endif

#ifdef HAVE_TOON
#include <cstring>
#include <TooN/TooN.h>
#include <TooN/se3.h>
#endif

namespace pangolin {

//! @brief Capture OpenGL matrix types in enum to typing
enum OpenGlStack {
    GlProjectionStack = GL_PROJECTION,
    GlModelViewStack = GL_MODELVIEW,
    GlTextureStack = GL_TEXTURE
};

enum AxisDirection
{
    AxisNone,
    AxisNegX, AxisX,
    AxisNegY, AxisY,
    AxisNegZ, AxisZ
};

struct CameraSpec {
    GLdouble forward[3];
    GLdouble up[3];
    GLdouble right[3];
    GLdouble img_up[2];
    GLdouble img_right[2];
};

const static CameraSpec CameraSpecOpenGl = {{0,0,-1},{0,1,0},{1,0,0},{0,1},{1,0}};
const static CameraSpec CameraSpecYDownZForward = {{0,0,1},{0,-1,0},{1,0,0},{0,-1},{1,0}};

// Direction vector for each AxisDirection enum
const static GLdouble AxisDirectionVector[7][3] = {
    {0,0,0},
    {-1,0,0}, {1,0,0},
    {0,-1,0}, {0,1,0},
    {0,0,-1}, {0,0,1}
};

//! @brief Object representing OpenGl Matrix
struct OpenGlMatrix {
    static OpenGlMatrix Translate(double x, double y, double z);
    static OpenGlMatrix Scale(double x, double y, double z);
    
    OpenGlMatrix();
    
#ifdef HAVE_EIGEN
    template<typename P>
    OpenGlMatrix(const Eigen::Matrix<P,4,4>& mat);
    
    template<typename P>
    operator Eigen::Matrix<P,4,4>() const;
#endif // HAVE_EIGEN

#ifdef HAVE_TOON
    OpenGlMatrix(const TooN::SE3<>& T);
    OpenGlMatrix(const TooN::Matrix<4,4>& M);
    operator const TooN::SE3<>() const;
    operator const TooN::Matrix<4,4>() const;
#endif // HAVE_TOON    
    
    // Load matrix on to OpenGl stack
    void Load() const;
    
    void Multiply() const;
    
    void SetIdentity();
    
    OpenGlMatrix Transpose() const;
    
    OpenGlMatrix Inverse() const;
    
    // Column major Internal buffer
    GLdouble m[16];
};

OpenGlMatrix operator*(const OpenGlMatrix& lhs, const OpenGlMatrix& rhs);
std::ostream& operator<<(std::ostream& os, const OpenGlMatrix& mat);

//! @brief deprecated
struct OpenGlMatrixSpec : public OpenGlMatrix {
    // Specify which stack this refers to
    OpenGlStack type;
};

//! @brief Object representing attached OpenGl Matrices / transforms
//! Only stores what is attached, not entire OpenGl state (which would
//! be horribly slow). Applying state is efficient.
class OpenGlRenderState
{
public:
    OpenGlRenderState();
    OpenGlRenderState(const OpenGlMatrix& projection_matrix);
    OpenGlRenderState(const OpenGlMatrix& projection_matrix, const OpenGlMatrix& modelview_matrix);
    
    static void ApplyIdentity();
    
    void Apply() const;
    OpenGlRenderState& SetProjectionMatrix(OpenGlMatrix spec);
    OpenGlRenderState& SetModelViewMatrix(OpenGlMatrix spec);
    
    OpenGlMatrix& GetProjectionMatrix();
    OpenGlMatrix GetProjectionMatrix() const;
    
    OpenGlMatrix& GetModelViewMatrix();
    OpenGlMatrix GetModelViewMatrix() const;
    
    OpenGlMatrix GetProjectionModelViewMatrix() const;
    OpenGlMatrix GetProjectiveTextureMatrix() const;
    
    void EnableProjectiveTexturing() const;
    void DisableProjectiveTexturing() const;
    
    //! Seemlessly move OpenGl camera relative to changes in T_wc,
    //! whilst still enabling interaction
    void Follow(const OpenGlMatrix& T_wc, bool follow = true);
    void Unfollow();
    
    PANGOLIN_DEPRECATED
    OpenGlRenderState& Set(OpenGlMatrixSpec spec);
    
protected:
    std::map<OpenGlStack,OpenGlMatrix> stacks;
    OpenGlMatrix T_cw;
    bool follow;
};

OpenGlMatrixSpec ProjectionMatrixRUB_BottomLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );
OpenGlMatrixSpec ProjectionMatrixRDF_TopLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );
OpenGlMatrixSpec ProjectionMatrixRDF_BottomLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );

//! Use OpenGl's default frame RUB_BottomLeft
OpenGlMatrixSpec ProjectionMatrix(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );
OpenGlMatrixSpec ProjectionMatrixOrthographic(double l, double r, double b, double t, double n, double f );

//! Generate glulookat style model view matrix, looking at (lx,ly,lz)
//! X-Right, Y-Up, Z-Back
OpenGlMatrix ModelViewLookAtRUB(double ex, double ey, double ez, double lx, double ly, double lz, double ux, double uy, double uz);

//! Generate glulookat style model view matrix, looking at (lx,ly,lz)
//! X-Right, Y-Down, Z-Forward
OpenGlMatrix ModelViewLookAtRDF(double ex, double ey, double ez, double lx, double ly, double lz, double ux, double uy, double uz);

//! Generate glulookat style model view matrix, OpenGL Default camera convention (XYZ=RUB), looking at (lx,ly,lz)
OpenGlMatrix ModelViewLookAt(double x, double y, double z, double lx, double ly, double lz, AxisDirection up);
OpenGlMatrix ModelViewLookAt(double ex, double ey, double ez, double lx, double ly, double lz, double ux, double uy, double uz);

OpenGlMatrix IdentityMatrix();
OpenGlMatrixSpec IdentityMatrix(OpenGlStack type);
OpenGlMatrixSpec negIdentityMatrix(OpenGlStack type);

#ifdef HAVE_TOON
OpenGlMatrixSpec FromTooN(const TooN::SE3<>& T_cw);
OpenGlMatrixSpec FromTooN(OpenGlStack type, const TooN::Matrix<4,4>& M);
TooN::Matrix<4,4> ToTooN(const OpenGlMatrixSpec& ms);
TooN::SE3<> ToTooN_SE3(const OpenGlMatrixSpec& ms);
#endif

}

// Inline definitions
namespace pangolin
{
inline OpenGlMatrix::OpenGlMatrix() {
}

#ifdef HAVE_EIGEN
template<typename P> inline
OpenGlMatrix::OpenGlMatrix(const Eigen::Matrix<P,4,4>& mat)
{
    for(int r=0; r<4; ++r ) {
        for(int c=0; c<4; ++c ) {
            m[c*4+r] = mat(r,c);
        }
    }
}

template<typename P>
OpenGlMatrix::operator Eigen::Matrix<P,4,4>() const
{
    Eigen::Matrix<P,4,4> mat;
    for(int r=0; r<4; ++r ) {
        for(int c=0; c<4; ++c ) {
            mat(r,c) = m[c*4+r];
        }
    }
    return mat;
}
#endif

#ifdef HAVE_TOON
inline OpenGlMatrix::OpenGlMatrix(const TooN::SE3<>& T)
{
    TooN::Matrix<4,4,double,TooN::ColMajor> M;
    M.slice<0,0,3,3>() = T.get_rotation().get_matrix();
    M.T()[3].slice<0,3>() = T.get_translation();
    M[3] = TooN::makeVector(0,0,0,1);
    std::memcpy(m, &(M[0][0]),16*sizeof(double));
}

inline OpenGlMatrix::OpenGlMatrix(const TooN::Matrix<4,4>& M)
{
    // Read in remembering col-major convension for our matrices
    int el = 0;
    for(int c=0; c<4; ++c)
        for(int r=0; r<4; ++r)
            m[el++] = M[r][c];    
}

inline OpenGlMatrix::operator const TooN::SE3<>() const
{
    const TooN::Matrix<4,4> m = *this;
    const TooN::SO3<> R(m.slice<0,0,3,3>());
    const TooN::Vector<3> t = m.T()[3].slice<0,3>();
    return TooN::SE3<>(R,t);    
}

inline OpenGlMatrix::operator const TooN::Matrix<4,4>() const
{
    TooN::Matrix<4,4> M;
    int el = 0;
    for( int c=0; c<4; ++c )
        for( int r=0; r<4; ++r )
            M(r,c) = m[el++];
    return M;
}

PANGOLIN_DEPRECATED
inline OpenGlMatrixSpec FromTooN(const TooN::SE3<>& T_cw)
{
    TooN::Matrix<4,4,double,TooN::ColMajor> M;
    M.slice<0,0,3,3>() = T_cw.get_rotation().get_matrix();
    M.T()[3].slice<0,3>() = T_cw.get_translation();
    M[3] = TooN::makeVector(0,0,0,1);
    
    OpenGlMatrixSpec P;
    P.type = GlModelViewStack;
    std::memcpy(P.m, &(M[0][0]),16*sizeof(double));
    return P;
}

PANGOLIN_DEPRECATED
inline OpenGlMatrixSpec FromTooN(OpenGlStack type, const TooN::Matrix<4,4>& M)
{
    // Read in remembering col-major convension for our matrices
    OpenGlMatrixSpec P;
    P.type = type;
    int el = 0;
    for(int c=0; c<4; ++c)
        for(int r=0; r<4; ++r)
            P.m[el++] = M[r][c];
    return P;
}

PANGOLIN_DEPRECATED
inline TooN::Matrix<4,4> ToTooN(const OpenGlMatrix& ms)
{
    TooN::Matrix<4,4> m;
    int el = 0;
    for( int c=0; c<4; ++c )
        for( int r=0; r<4; ++r )
            m(r,c) = ms.m[el++];
    return m;
}

PANGOLIN_DEPRECATED
inline TooN::SE3<> ToTooN_SE3(const OpenGlMatrix& ms)
{
    TooN::Matrix<4,4> m = ms;
    const TooN::SO3<> R(m.slice<0,0,3,3>());
    const TooN::Vector<3> t = m.T()[3].slice<0,3>();
    return TooN::SE3<>(R,t);
}

#endif

}

#endif // PANGOLIN_OPENGLRENDERSTATE_H
