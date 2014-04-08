#ifndef OCULUS_HUD_H
#define OCULUS_HUD_H

#include <pangolin/compat/memory.h>
#include <pangolin/view.h>
#include <pangolin/gl.h>
#include <pangolin/glsl.h>
#include <OVR.h>

namespace pangolin
{

class OculusHud : public View
{
public:
    OculusHud();

    void Render();
    void RenderFramebuffer();

    unsigned int NumViews();
    void Bind();
    void RenderToView(int view);
    void Unbind();

    OpenGlMatrix HeadTransform();

protected:
    // Shader with lens distortion and chromatic aberration correction.
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

    OpenGlMatrix headTransform;
    OpenGlMatrix projection[2];
    OpenGlMatrix viewAdjust[2];
    Viewport viewport[2];
};

}

#endif // OCULUS_HUD_H
