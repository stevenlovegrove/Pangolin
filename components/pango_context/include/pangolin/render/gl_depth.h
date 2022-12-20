#pragma once

namespace pangolin
{

// Scales a depth value `real_depth`, in world units, to the range [0, 1]
// Derived from https://stackoverflow.com/questions/6652253
float glDepthFromReal(float real_depth, float znear, float zfar)
{
  float const A = -2.0 * znear * zfar;
  float const B = zfar - znear;
  float const C = zfar + znear;

  // Scale to normalized device coordinates, [-1, 1]
  float const z_n = (A / real_depth + C) / B;

  // Scale to the range [0, 1]
  float z_b = (z_n + 1.0) / 2.0;

  return z_b;
}

float realDepthFromGl(float gl_depth, float znear, float zfar)
{
  // Scale to normalized device coordinates, [-1, 1]
  float const A = -2.0 * znear * zfar;
  float const B = zfar - znear;
  float const C = zfar + znear;

  // Scale to normalized device coordinates, [-1, 1] from [0,1]
  float const z_n = 2.0 * gl_depth - 1.0;

  // Scale to linear depth range
  float const real_depth = A / (B * z_n - C);

  return real_depth;
}

}  // namespace pangolin
