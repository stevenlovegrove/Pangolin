@start vertex
#version 150 core
out vec2 v_tex;

const vec2 pos[4] = vec2[4](
  vec2(-1.0, 1.0), vec2(-1.0,-1.0),
  vec2( 1.0, 1.0), vec2( 1.0,-1.0)
);

void main()
{
  v_tex = 0.5*vec2(1.0,-1.0)*pos[gl_VertexID] + vec2(0.5);
  gl_Position=vec4(pos[gl_VertexID], 0.0, 1.0);
}

@start fragment
#version 150 core
in vec2 v_tex;
uniform sampler2D image;
// uniform bool gray_only;

out vec4 color;
void main()
{
//   if(gray_only) {
    // color=vec4(vec3(texture(image, v_tex).r), 1.0);
//   }else{
    color=texture(image, v_tex);
//   }
}
