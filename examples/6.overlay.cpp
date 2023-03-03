#include <pangolin/context/context.h>
#include <pangolin/gl/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/layer/all_layers.h>
#include <pangolin/video/video.h>

/*
  == Pangolin-by-example ==

*/

using namespace pangolin;
using namespace sophus;

// Forward declaration. See end of this file for the inline shader code.
extern const char* eg_shader;

struct ExampleCustomLayer : public Layer {
  std::string name() const override { return "example"; }

  Size sizeHint() const override { return size_; }

  void renderIntoRegion(const Context& c, const RenderParams& p) override
  {
    c.setViewport(p.region);
    auto bind_prog = prog->bind();
    auto bind_vao = vao.bind();
    u_time = std::fmod(u_time.getValue() + 0.01, 1.0);
    PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
  }

  GlVertexArrayObject vao = {};
  Shared<GlSlProgram> prog =
      GlSlProgram::Create({.sources = {{.glsl_code = eg_shader}}});
  GlUniform<float> u_time = {prog, "time"};
  Size size_ = {Parts{1}, Parts{1}};
};

int main(int argc, char** argv)
{
  PANGO_ENSURE(argc == 2, "Please provide one argument - the URL to a video.");
  const std::string video_url = argv[1];

  auto context = Context::Create({
      .title = "Pangolin Video",
      .window_size = {640, 480},
  });

  auto video_input = OpenVideo(video_url);
  auto video_view = DrawnImage::Create({});
  auto custom = Shared<ExampleCustomLayer>::make();

  // Stack the custom layer ontop of the video layer
  context->setLayout(custom ^ video_view);

  context->loop([&]() {
    if (auto maybe_image = optGet(video_input->GrabImages(), 0)) {
      video_view->image->update(*maybe_image);
    }
    return true;
  });
  return 0;
}

// Beautiful shader from https://www.shadertoy.com/view/WdB3Dw
const char* eg_shader = R"SHADER(
@start vertex
#version 150 core
out vec2 v_tex;
const vec2 pos[4] = vec2[4](
  vec2(-1.0, 1.0), vec2(-1.0,-1.0),
  vec2( 1.0, 1.0), vec2( 1.0,-1.0)
);
void main() {
  v_tex = 0.5*vec2(1.0,-1.0)*pos[gl_VertexID] + vec2(0.5);
  gl_Position=vec4(pos[gl_VertexID], 0.0, 1.0);
}
@start fragment
#version 150 core
in vec2 v_tex;
out vec4 color_out;
uniform float time;
uniform vec2 iResolution;

#define PI 3.14159265359
void pR(inout vec2 p, float a) { p = cos(a)*p + sin(a)*vec2(p.y, -p.x); }
float smax(float a, float b, float r) {
    vec2 u = max(vec2(r + a,r + b), vec2(0));
    return min(-r, max (a, b)) + length(u);
}
vec3 pal( in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d ) {
    return a + b*cos( 6.28318*(c*t+d) );
}
vec3 spectrum(float n) {
    return pal( n, vec3(0.5,0.5,0.5),vec3(0.5,0.5,0.5),vec3(1.0,1.0,1.0),vec3(0.0,0.33,0.67) );
}
vec4 inverseStereographic(vec3 p, out float k) {
    k = 2.0/(1.0+dot(p,p)); return vec4(k*p,k-1.0);
}
float fTorus(vec4 p4) {
    float d1 = length(p4.xy) / length(p4.zw) - 1.;
    float d2 = length(p4.zw) / length(p4.xy) - 1.;
    float d = d1 < 0. ? -d1 : d2; d /= PI; return d;
}
float fixDistance(float d, float k) {
    float sn = sign(d); d = abs(d); d = d / k * 1.82; d += 1.; d = pow(d, .5);
    d -= 1.; d *= 5./3.; d *= sn; return d;
}
float map(vec3 p) {
    float k; vec4 p4 = inverseStereographic(p,k);
    pR(p4.zy, time * -PI / 2.); pR(p4.xw, time * -PI / 2.);
    float d = fTorus(p4); d = abs(d); d -= .2; d = fixDistance(d, k);
    d = smax(d, length(p) - 1.85, .2); return d;
}
mat3 calcLookAtMatrix(vec3 ro, vec3 ta, vec3 up) {
    vec3 ww = normalize(ta - ro);
    vec3 uu = normalize(cross(ww,up));
    vec3 vv = normalize(cross(uu,ww));
    return mat3(uu, vv, ww);
}
void main() {
    vec3 camPos = vec3(1.8, 5.5, -5.5) * 1.75;
    vec3 camTar = vec3(.0,0,.0); vec3 camUp = vec3(-1,0,-1.5);
    mat3 camMat = calcLookAtMatrix(camPos, camTar, camUp);
    float focalLength = 5.; vec2 p = v_tex*4.0 - vec2(1.0);
    vec3 rayDirection = normalize(camMat * vec3(p, focalLength));
    vec3 rayPosition = camPos; float rayLength = 0.;
    float distance = 0.; vec3 color = vec3(0); vec3 c;
    const float ITER = 82.; const float FUDGE_FACTORR = .8;
    const float INTERSECTION_PRECISION = .001;
    const float MAX_DIST = 20.; float min_dist = 1e12;
    for (float i = 0.; i < ITER; i++) {
        rayLength += max(INTERSECTION_PRECISION, abs(distance) * FUDGE_FACTORR);
        rayPosition = camPos + rayDirection * rayLength;
        distance = map(rayPosition);
        min_dist = min(min_dist, distance);
        c = vec3(max(0., .01 - abs(distance)) * .5);
        c *= vec3(1.4,2.1,1.7); // blue green tint
        c += vec3(.6,.25,.7) * FUDGE_FACTORR / 160.;
        c *= smoothstep(20., 7., length(rayPosition));
        float rl = smoothstep(MAX_DIST, .1, rayLength);
        c *= rl; c *= spectrum(rl * 6. - .6); color += c;
        if (rayLength > MAX_DIST) break;
    }
    color = pow(color, vec3(1. / 1.8)) * 2.;
    color = pow(color, vec3(2.)) * 3.;
    color = pow(color, vec3(1. / 2.2));
    float alpha = clamp( 1.0 - min_dist*min_dist, 0.0, 1.0);
    color_out = vec4(color, alpha);
}
)SHADER";
