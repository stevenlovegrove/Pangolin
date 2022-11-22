@start vertex
#version 150 core

uniform vec2 viewport_size;

out vec2 pix;

const vec2 pos[4] = vec2[4](
  vec2( -1.0, +1.0), vec2( -1.0, -1.0),
  vec2( +1.0, +1.0), vec2( +1.0, -1.0)
);

void main()
{
  vec2 ndc = pos[gl_VertexID];

  pix = (ndc + vec2(1.0))/2.0 * viewport_size;
  gl_Position = vec4(ndc, 0.0, 1.0);
}

@start fragment
#version 150 core

in vec2 pix;

uniform vec4 color1;
uniform vec4 color2;
uniform int checksize;

out vec4 color;

void main()
{
  bool x = (int(pix.x)/checksize % 2) == 0;
  bool y = (int(pix.y)/checksize % 2) == 0;
  bool check = x ^^ y;
  color = check ? color1 : color2;
  // color = vec4(1.0,0.0,0.0,1.0);
}
