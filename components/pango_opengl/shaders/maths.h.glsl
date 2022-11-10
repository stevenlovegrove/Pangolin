#define PI 3.1415926538

bool isFinite(float v)
{
    return v == v;
}

// Maps `value` from the range [min1, max1] to the range [min2, max2]
float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

float gamma(float val, float gamma_val) {
  return pow(val, gamma_val);
}

vec2 gamma(vec2 val, float gamma_val) {
  return vec2(
      gamma(val.x, gamma_val),
      gamma(val.y, gamma_val)
  );
}

vec3 gamma(vec3 val, float gamma_val) {
  return vec3(
      gamma(val.x, gamma_val),
      gamma(val.y, gamma_val),
      gamma(val.z, gamma_val)
  );
}

vec4 gamma(vec4 val, float gamma_val) {
  return vec4(
      gamma(val.x, gamma_val),
      gamma(val.y, gamma_val),
      gamma(val.z, gamma_val),
      gamma(val.w, gamma_val)
  );
}

// Returns the lowest, finite, positive number out of 'a' and 'b'
// If neither 'a' or 'b' are finite, result is also not finite
float smallestPositive(float a, float b) {
  return a < 0 ? b : (b < 0 ? a : (min(a, b)));
}

// Returns the lowest, finite, positive number out of 'a' and 'b'
// If neither 'a' or 'b' are finite, result is also not finite
float largestNegative(float a, float b) {
  return a > 0 ? b : (b > 0 ? a : (max(a, b)));
}

// Returns the two intersections of `ray` and `sphere`, as distances from the
// ray_origin, or NAN if none.
vec2 sphereIntersect(vec3 ray_origin, vec3 ray_dir, vec3 sphere_center,
                      float sphere_radius) {
  vec3 A = ray_origin - sphere_center;
  float a = dot(ray_dir, ray_dir);
  float b = 2.0 * dot(A, ray_dir);
  float c = dot(A, A) - sphere_radius * sphere_radius;

  float bb_m_4ac = b * b - 4 * a * c;
  if (bb_m_4ac > 0) {
    float d1 = (-b - sqrt(bb_m_4ac)) / 2 * a;
    float d2 = (-b + sqrt(bb_m_4ac)) / 2 * a;
    return vec2(d1, d2);
  } else {
    // We use NAN on miss.
    return vec2(0.0 / 0.0, 0.0 / 0.0);
  }
}

vec2 reciprocal(vec2 v)
{
    return vec2(1.0 / v.x, 1.0 / v.y);
}

mat3 affineTranslation(vec2 t)
{
    return mat3(
        vec3(1, 0, 0),     // col 0
        vec3(0, 1, 0),     // col 1
        vec3(t.x, t.y, 1)  // col 2
    );
}

// th is rotation clockwise(?) in radians
mat3 affineRotate(float th)
{
    float ct = cos(th);
    float st = sin(th);

    return mat3(
        vec3(ct, -st, 0),     // col 0
        vec3(-st, ct, 0),     // col 1
        vec3(0, 0, 1)  // col 2
    );
}

mat3 affineScale(vec2 s)
{
    return mat3(
        vec3(s.x, 0, 0),     // col 0
        vec3(0, s.y, 0),     // col 1
        vec3(0, 0, 1)  // col 2
    );
}

mat3 affineTranspose()
{
    return mat3(
        vec3(0.0, 1.0, 0.0), // col 0
        vec3(1.0, 0.0, 0.0), // col 1
        vec3(0.0, 0.0, 1.0)  // col 2
    );
}

mat3 affineScale(float s)
{
    return affineScale(vec2(s,s));
}

mat4 poseInverse(mat4 T_ba)
{
    mat3 R_ab = transpose(mat3(T_ba));
    vec3 a_b = T_ba[3].xyz;
    vec3 b_a =  -R_ab * a_b;

    mat4 T_ab = mat4(
        vec4(R_ab[0], 0.0), // col 0
        vec4(R_ab[1], 0.0), // col 1
        vec4(R_ab[2], 0.0), // col 2
        vec4(b_a, 1.0)      // col 3
    );
    return T_ab;
}
