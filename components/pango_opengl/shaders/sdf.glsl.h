float sdf_circ(vec2 p, vec2 center, float rad)
{
    float dist = length(p - center);
    return dist - rad;
}

float sdf_rect(vec2 p, vec2 center, vec2 half_size) {
  vec2 d = abs(p - center) - half_size;
  float outside = length(max(d, 0.));
  float inside = min(max(d.x, d.y), 0.);
  return outside + inside;
}

float sdf_rounded_rect(vec2 p, vec2 center, vec2 half_size, float rad) {
    return sdf_rect(p,center,half_size-vec2(rad)) - rad;
}

float sdf_rect_extent(vec2 p, vec2 corner1, vec2 corner2) {
    vec2 center = (corner1 + corner2) / 2.0;
    vec2 half_size = abs(corner2 - corner1) / 2.0;
    return sdf_rect(p, center, half_size );
}

float sdf_rounded_rect_extent(vec2 p, vec2 corner1, vec2 corner2, float rad) {
    vec2 center = (corner1 + corner2) / 2.0;
    vec2 half_size = abs(corner2 - corner1) / 2.0;
    return sdf_rect(p,center,half_size-vec2(rad)) - rad;
}

float sdf_line_segment(vec2 p, vec2 a, vec2 b) {
    vec2 ba = b - a;
    vec2 pa = p - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
    return length(pa - h * ba);
}

float sdf_grid(vec2 p, vec2 spacing)
{
  vec2 half_space = spacing / 2.0;
  vec2 dist2 = mod(p + half_space, spacing) - half_space;
  return length(dist2);
}

float sdf_grid(vec2 p_in, vec2 spacing, vec2 origin, ivec2 grid_size)
{
  vec2 p = p_in - origin;
  vec2 half_space = spacing / 2.0;
  vec2 dist2 = mod(p + half_space, spacing) - half_space;
  ivec2 n = ivec2( (p / spacing) - 0.5);
  if( all(lessThanEqual(ivec2(0,0),n)) && all(lessThan(n,grid_size)) ) {
    return length(dist2);
  }else{
    return 0.0/0.0;
  }
}

float opacity(float sdf)
{
    return clamp(-sdf + 0.5, 0.0, 1.0);
}

vec4 color_sdf(float sdf, vec3 color_inside)
{
    return vec4(color_inside, smoothstep(-0.5, 0.5, -sdf));
}

vec4 color_sdf(float sdf, vec4 color_inside)
{
    return vec4(color_inside.xyz, color_inside.w * smoothstep(-0.5, 0.5, -sdf));
}

vec3 transition(vec3 a, vec3 b, float point, float x)
{
    float s = smoothstep(point-0.5, point+0.5, x);
    return (1.0-s)*a + s*b;
}

// Border encroaches toward interior
vec4 color_sdf(float sdf, vec3 color_inside, vec3 color_border, float border_size)
{
    return vec4(
        transition(color_inside, color_border, -border_size, sdf),
        smoothstep(-0.5, 0.5, -sdf)
    );
}
