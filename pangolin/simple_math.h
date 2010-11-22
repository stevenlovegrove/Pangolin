#ifndef PANGOLIN_SIMPLE_MATH_H
#define PANGOLIN_SIMPLE_MATH_H

#include <iostream>
#include <string.h>
#include <algorithm>
#include <stdarg.h>

namespace pangolin
{

static const float Identity3[] = {1,0,0, 0,1,0, 0,0,1};
static const float Zero3[]     = {0,0,0, 0,0,0, 0,0,0};
static const float Identity4[] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
static const float Zero4[]     = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};

// m = m1 + m2
template<int R, int C, typename P>
void MatPrint(const P m[R*C])
{
  for( int r=0; r < R; ++r)
  {
    for( int c=0; c < C; ++c )
      std::cout << m[R*c+r] << " ";
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

// Set array using varadic arguments
template<int R, int C, typename P>
void MatSet(P m[R*C], ...)
{
  va_list ap;
  va_start(ap,m);
  for( int i=0; i< R*C; ++i )
  {
    *m = (P)va_arg(ap,double);
    ++m;
  }
}

// m = zeroes(N)
template<int N, typename P>
void SetZero(P m[N*N] )
{
  memset(m,0,N*N);
}

// m = identity(N)
template<int N, typename P>
void SetIdentity(P m[N*N] )
{
  memset(m,0,N*N);
  for( int i=0; i< N; ++i )
    m[N*i+i] = 1;
}

// mo = m1 * m2
template<int R, int M, int C, typename P>
void MatMul(P mo[R*C], const P m1[R*M], const P m2[M*C] )
{
  for( int r=0; r < R; ++r)
    for( int c=0; c < C; ++c )
    {
      mo[R*c+r] = 0;
      for( int m=0; m < M; ++ m) mo[R*c+r] += m1[R*m+r] * m2[M*c+m];
    }
}

// mo = m1 * transpose(m2)
template<int R, int M, int C, typename P>
void MatMulTranspose(P mo[R*C], const P m1[R*M], const P m2[C*M] )
{
  for( int r=0; r < R; ++r)
    for( int c=0; c < C; ++c )
    {
      mo[R*c+r] = 0;
      for( int m=0; m < M; ++ m) mo[R*c+r] += m1[R*m+r] * m2[C*m+c];
    }
}

// m = m1 + m2
template<int R, int C, typename P>
void MatAdd(P m[R*C], const P m1[R*C], const P m2[R*C])
{
  for( int i=0; i< R*C; ++i )
    m[i] = m1[i] + m2[i];
}

// m = m1 + m2
template<int R, int C, typename P>
void MatSub(P m[R*C], const P m1[R*C], const P m2[R*C])
{
  for( int i=0; i< R*C; ++i )
    m[i] = m1[i] - m2[i];
}

// m = m1 + m2
template<int R, int C, typename P>
void MatMul(P m[R*C], const P m1[R*C], P scalar)
{
  for( int i=0; i< R*C; ++i )
    m[i] = m1[i] * scalar;
}

// m = m1 + m2
template<int R, int C, typename P>
void MatMul(P m[R*C], P scalar)
{
  for( int i=0; i< R*C; ++i )
    m[i] *= scalar;
}

template<int N, typename P>
void MatTranspose(P out[N*N], const P in[N*N] )
{
  for( int c=0; c<N; ++c )
    for( int r=0; r<N; ++r )
      out[N*c+r] = in[N*r+c];
}

template<int N, typename P>
void MatTranspose(P m[N*N] )
{
  for( int c=0; c<N; ++c )
    for( int r=0; r<c; ++r )
      std::swap<P>(m[N*c+r],m[N*r+c]);
}

// s = skewSymetrixMatrix(v)
template<typename P>
void MatSkew(P s[3*3], const P v[3] )
{
  s[0] = 0;
  s[1] = v[2];
  s[2] = -v[1];
  s[3] = -v[2];
  s[4] = 0;
  s[5] = v[0];
  s[6] = v[1];
  s[7] = -v[0];
  s[8] = 0;
}

template<int N, typename P>
void MatOrtho( P m[N*N] )
{
  P Itimes3[N*N];
  SetIdentity<N>(Itimes3);
  MatMul<N,N>(Itimes3,(P)3.0);

  P mmT[N*N];
  MatMulTranspose<N,N,N>(mmT,m,m);

  MatSub<N,N>(m,Itimes3,mmT);
  MatMul<N,N>(m,(P)0.5);
}

template<typename P>
void LieSetIdentity(P T_ba[3*4] )
{
  SetIdentity<3>(T_ba);
  memset(T_ba+(3*3),0.0,3);
}

template<typename P>
void LieSetSE3(P T_ba[3*4], const P R_ba[3*3], const P a_b[3] )
{
  LieSetRotation(T_ba,R_ba);
  memcpy(T_ba+(3*3),a_b,3);
}

template<typename P>
void LieGetRotation(P R_ba[3*3], const P T_ba[3*4] )
{
  memcpy(R_ba,T_ba,3*3);
}

template<typename P>
void LieSetRotation(P T_ba[3*4], const P R_ba[3*3] )
{
  memcpy(T_ba,R_ba,3*3);
}

template<typename P>
void LieSetTranslation(P T_ba[3*4], const P a_b[3*3] )
{
  memcpy(T_ba+(3*3),a_b,3);
}

template<typename P>
void LieApplySO3( P out[3], const P R_ba[3*3], const P in[3] )
{
  MatMul<3,3,3,P>(out,R_ba,in);
}

template<typename P>
void LieApplySE3vec( P x_b[3], const P T_ba[3*4], const P x_a[3] )
{
  P rot[3];
  MatMul<3,3,3,P>(rot,T_ba,x_a);
  MatAdd<3,1,P>(x_b,rot,T_ba+(3*3));
}

template<typename P>
void LieMulSO3( P R_ca[3*3], const P R_cb[3*3], const P R_ba[3*3] )
{
  MatMul<3,3,3>(R_ca,R_cb,R_ba);
}

template<typename P>
void LieMulSE3( P T_ca[3*4], const P T_cb[3*4], const P T_ba[3*4] )
{
  LieMulSO3<>(T_ca,T_cb,T_ba);
  P R_cb_times_a_b[3];
  LieApplySO3<>(R_cb_times_a_b,T_cb,T_ba+(3*3));
  MatAdd<3,3>(T_ca+(3*3),R_cb_times_a_b,T_cb+(3*3));
}

template<typename P>
void LiePutSE3in4x4(P out[4*4], const P in[3*4] )
{
  SetIdentity<4>(out);
  memcpy(out,in,3);
  memcpy(out+4,in+3,3);
  memcpy(out+8,in+6,3);
  memcpy(out+12,in+9,3);
}

template<typename P>
void LieSE3from4x4(P out[3*4], const P in[4*4] )
{
  memcpy(out,in,3);
  memcpy(out+3,in+4,3);
  memcpy(out+6,in+8,3);
  memcpy(out+9,in+12,3);
}

template<typename P>
void LieMul4x4bySE3( P T_ca[4*4], const P T_cb[3*4], const P T_ba[4*4] )
{
  // TODO: fast
  P lT_ba[3*4];
  LieSE3from4x4<>(lT_ba,T_ba);
  P lT_ca[3*4];
  LieMulSE3<>(lT_ca,T_cb,lT_ba);
  LiePutSE3in4x4(T_ca,lT_ca);
}

template<typename P>
void LieTransposeSO3( P R_ab[3*3], const P R_ba[3*3] )
{
  MatTranspose<3,P>(R_ab,R_ba);
}

template<typename P>
void LieTransposeSE3( P T_ab[3*4], const P T_ba[3*4] )
{
  LieTransposeSO3<P>(T_ab,T_ba);
  P minus_b_a[3];
  LieApplySO3(minus_b_a, T_ab, T_ba+(3*3));
  MatMul<3,1,P>(T_ab+(3*3),minus_b_a, -1);
}

}

#endif //PANGOLIN_SIMPLE_MATH_H
