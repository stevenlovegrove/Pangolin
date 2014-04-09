#ifndef OCULUS_HUD_H
#define OCULUS_HUD_H

#include <pangolin/compat/memory.h>
#include <pangolin/view.h>
#include <pangolin/gl.h>
#include <pangolin/glsl.h>
#include <pangolin/handler_glbuffer.h>

#include <OVR.h>

namespace pangolin
{

class OculusHud : public View
{
public:
    OculusHud();

    void Render();
    void RenderFramebuffer();

    void SetHandler(Handler *handler);

    OpenGlMatrix HeadTransform();
    OpenGlMatrix HeadTransformDelta();
    pangolin::GlFramebuffer& Framebuffer();
    pangolin::OpenGlRenderState& DefaultRenderState();

    void UnwarpPoint(unsigned int view, const float in[2], float out[2]);

protected:
    // Oculus SDK Shader for applying lens and chromatic distortion.
    static const char* PostProcessFullFragShaderSrc;

    void InitialiseOculus();
    void InitialiseFramebuffer();
    void InitialiseShader();

    pangolin::GlTexture colourbuffer;
    pangolin::GlRenderBuffer depthbuffer;
    pangolin::GlFramebuffer framebuffer;
    pangolin::GlSlProgram occ;

    OVR::Ptr<OVR::DeviceManager> pManager;
    OVR::Ptr<OVR::HMDDevice> pHMD;
    OVR::Ptr<OVR::SensorDevice> pSensor;
    boostd::unique_ptr<OVR::SensorFusion> pFusionResult;
    OVR::HMDInfo HMD;
    OVR::Util::Render::StereoConfig stereo;

    View eyeview[2];
    pangolin::OpenGlRenderState default_cam;

    // Center to Oculus transform
    OpenGlMatrix T_oc;
};

struct Handler3DOculus : public pangolin::Handler3D
{
    Handler3DOculus(OculusHud& oculus, pangolin::OpenGlRenderState& cam_state, pangolin::AxisDirection enforce_up=pangolin::AxisNone, float trans_scale=0.01f);
    void GetPosNormal(pangolin::View& view, int x, int y, double p[3], double Pw[3], double Pc[3], double /*n*/[3], double default_z);

protected:
    OculusHud& oculus;
};

}

#endif // OCULUS_HUD_H
