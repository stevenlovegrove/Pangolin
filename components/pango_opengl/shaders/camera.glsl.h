// Returns the (u,v) coordinate in the discrete output image
// coordinate system of size 'output_image_dim'.
//  (0, 0) references center of bottom-left pixel
// tex is a continous coord representing viewport position in the range [0,1]
vec2 getCameraPixelCoord(vec2 output_image_dim, vec2 tex) {
  vec2 cam_continuous = output_image_dim * tex;

  // Convert to discrete pixel coordinates
  vec2 cam = cam_continuous - vec2(0.5, 0.5);
  return cam;
}

// Returns the (u,v) coordinate in the discrete output image
// coordinate system of size 'output_image_dim'.
//  (0, 0) references center of bottom-left pixel
// viewport_xy:  The top-left of the viewport in window coordinates
// viewport_dim: The dimensions of the viewport representing the output image
vec2 getCameraPixelCoord(vec2 output_image_dim, vec2 viewport_xy, vec2 viewport_dim) {
  // Window-relative coordinate of fragment
  // Assumes a lower-left origin
  vec2 win = gl_FragCoord.xy;

  // Viewport-relative coordinate of fragment
  vec2 view = win - viewport_xy;

  // Camera image relative coordinate of fragment
  vec2 cam_continuous = output_image_dim * view / viewport_dim;

  // Convert to discrete pixel coordinates
  vec2 cam = cam_continuous - vec2(0.5, 0.5);
  return cam;
}

// Project the image coordinate `uv` to a 3D point in camera space
vec3 unproj(mat3 Kinv, vec2 uv) {
    return Kinv * vec3(uv, 1.0);
}

vec3 unproj(vec2 x)
{
    return vec3(x,1.0);
}

// Scales a depth value `real_depth`, in world units, to the range [0, 1]
// Derived from https://stackoverflow.com/questions/6652253
float fragDepthFromSceneDepth(float real_depth, float znear, float zfar) {
  // Scale to normalized device coordinates, [-1, 1]
  float z_n =
      -((2.0 * znear * zfar) / real_depth - zfar - znear) / (zfar - znear);

  // Scale to the range [0, 1]
  float z_b = (z_n + 1.0) / 2.0;

  return z_b;
}
