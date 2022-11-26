@start vertex
#version 330 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_colors;
layout(location = 2) in uint a_type;

out vec3 v_position;
out vec4 v_color;
out uint v_type;

void main() {
    v_position = a_position;
    v_color = a_colors;
    v_type = a_type;
}

@start geometry
#version 330
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 v_position[];
in vec4 v_color[];
in uint v_type[];

flat out uint type;
out vec2 uv;
out vec4 color;

uniform mat4 proj;
uniform mat4 cam_from_world;
uniform float size;

vec2 corners[4] = vec2[](
    vec2(-1.0,-1.0), vec2(+1.0,-1.0),
    vec2(-1.0,+1.0), vec2(+1.0,+1.0)
);

void main() {
    for(uint i=0u; i < 4u;  ++i) {
        color = v_color[0];
        type = v_type[0];
        uv = corners[i] * vec2(1.0,-1.0);
        vec4 center_cam = cam_from_world * vec4(v_position[0], 1.0);
        gl_Position = proj * (center_cam + vec4(0.5*size * corners[i],0.0, 0.0) );
        EmitVertex();
    }

    EndPrimitive();
}

@start fragment
#version 330 core

#include <inigo/sdf_2d.glsl.h>

flat in uint type;
in vec4 color;
in vec2 uv;

out vec4 FragColor;

vec4 color_sdf(float sdf, vec4 color_inside) {
    return vec4(color_inside.xyz, color_inside.w * smoothstep(-0.5, 0.5, -sdf));
}

// Ordinal needs to match drawn_primitives.h Shapes enum
uint kNumShapes=11u;
float sdForShape(vec2 p, float r, uint shape) {
    switch(shape) {
    case 0: return sdCircle(p, 0.8*r);
    case 1: return sdBox(p, 0.7*vec2(r,r));
    case 2: return sdRhombus(p, 0.9*vec2(r,r));
    case 3: return sdEquilateralTriangle(1.2*p/r+vec2(0.0,0.2));
    case 4: return sdPentagon(p, 0.75*r);
    case 5: return sdHexagon(p, 0.75*r);
    case 6: return sdHexagram(p, 0.45*r);
    case 7: return sdStar5(p, 0.9*r, 0.5);
    case 8: return 1.4*sdHeart(p/r/1.4+vec2(0.0,0.6));
    case 9: return sdRoundedX(p, 1.1*r, r/4.0);
    default: return sdBlobbyCross(1.7*p/r, 1.0) - 0.5;
    }
}

float sdForType(vec2 p, float r, uint type) {
    uint shape = type >= kNumShapes ? type - kNumShapes : type;
    float sd = sdForShape(p,r,shape);
    return type >= kNumShapes ? opOnion(sd, 0.08) : sd;
}

void main() {
    vec2 w2 = 1.0 / fwidth(uv);
    float w = max(w2.x, w2.y);
    float sd = w*sdForType(uv, 1.0, type);
    FragColor = color_sdf(sd, color);
    if(sd > 0.8) discard;
}
