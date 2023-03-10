#pragma once

namespace pangolin
{

// Scales a depth value `real_depth`, in world units, to the range [0, 1]
// Derived from https://stackoverflow.com/questions/6652253
inline double glDepthFromReal(double real_depth, double znear, double zfar)
{
  const double A = -2.0 * znear * zfar;
  const double B = zfar - znear;
  const double C = zfar + znear;

  // Scale to normalized device coordinates, [-1, 1]
  const double z_n = (A / real_depth + C) / B;

  // Scale to the range [0, 1]
  double z_b = (z_n + 1.0) / 2.0;

  return z_b;
}

inline double realDepthFromGl(double gl_depth, double znear, double zfar)
{
  // Scale to normalized device coordinates, [-1, 1]
  const double A = -2.0 * znear * zfar;
  const double B = zfar - znear;
  const double C = zfar + znear;

  // Scale to normalized device coordinates, [-1, 1] from [0,1]
  const double z_n = 2.0 * gl_depth - 1.0;

  // Scale to linear depth range
  const double denom = B * z_n - C;
  const double real_depth = A / denom;
  return real_depth;
}

}  // namespace pangolin
