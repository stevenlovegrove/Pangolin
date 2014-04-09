#include "OculusHud.h"

#include <pangolin/pangolin.h>

namespace pangolin
{

std::ostream& operator<<(std::ostream& s, const Viewport& v)
{
    s << v.l << "," << v.b << ": " << v.w << "x" << v.h;
    return s;
}

OculusHud::OculusHud()
{
    InitialiseOculus();
    InitialiseFramebuffer();
    InitialiseShader();

    // Make this the default fullscreen view
    DisplayBase().AddDisplay(*this);
    SetBounds(Attach::Pix(0),Attach::Pix(HMD.VResolution),Attach::Pix(0),Attach::Pix(HMD.HResolution));
    View::SetHandler(&pangolin::StaticHandler);

    // Initialise per-eye views / parameters
    for(int i=0; i<2; ++i) {
        OVR::Util::Render::StereoEyeParams eye =
                stereo.GetEyeRenderParams( OVR::Util::Render::StereoEye(1+i) );

        eyeview[i].SetBounds(Attach::Pix(eye.VP.y), Attach::Pix(eye.VP.y+eye.VP.h), Attach::Pix(eye.VP.x), Attach::Pix(eye.VP.x+eye.VP.w));
        this->AddDisplay(eyeview[i]);

        default_cam.GetProjectionMatrix(i) = eye.Projection;
        default_cam.GetViewOffset(i) = eye.ViewAdjust;
    }

    T_oc = IdentityMatrix();

    pangolin::SetFullscreen();
}

void OculusHud::SetHandler(Handler *handler)
{
    eyeview[0].SetHandler(handler);
    eyeview[1].SetHandler(handler);
}

OpenGlMatrix OculusHud::HeadTransform()
{
    const OVR::Quatf q = pFusionResult->GetOrientation();
    const OVR::Matrix4f T_oc = OVR::Matrix4f(q).Inverted();
    return OpenGlMatrix(T_oc);
}

OpenGlMatrix OculusHud::HeadTransformDelta()
{
    OpenGlMatrix T_2c = HeadTransform();
    OpenGlMatrix T_21 = T_2c * T_oc.Inverse();

    // Update
    T_oc = T_2c;

    return T_21;
}


pangolin::GlFramebuffer& OculusHud::Framebuffer()
{
    return framebuffer;
}

pangolin::OpenGlRenderState& OculusHud::DefaultRenderState()
{
    return default_cam;
}

void OculusHud::UnwarpPoint(unsigned int view, const float in[2], float out[2])
{
    const OVR::Util::Render::DistortionConfig& Distortion = stereo.GetDistortionConfig();
    pangolin::Viewport& vp = eyeview[view].v;
    const float x = vp.l / (float)v.w;
    const float y = vp.b / (float)v.h;
    const float w = vp.w / (float)v.w;
    const float h = vp.h / (float)v.h;

    float lensc[] = { x + (w + (1-view*2)*Distortion.XCenterOffset * 0.5f)*0.5f, y + h*0.5f };
    float scale[] = { (float)v.w, (float)v.h };

    float inc[] = {
        in[0] / scale[0] - lensc[0],
        in[1] / scale[1] - lensc[1],
    };

    const float r = std::sqrt(inc[0]*inc[0] + inc[1]*inc[1]);
    const float nr = const_cast<OVR::Util::Render::DistortionConfig&>(Distortion).DistortionFnInverse(r);

    float outc[] {
        nr * inc[0] / r,
        nr * inc[1] / r
    };

    out[0] = scale[0] * (outc[0] + lensc[0]);
    out[1] = scale[1] * (outc[1] + lensc[1]);
}

void OculusHud::RenderFramebuffer()
{    
    Activate();

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0,1.0,1.0);
    // Render framebuffer views to screen via distortion shader
    for(int i=0; i<2; ++i)
    {
        pangolin::Viewport& vp = eyeview[i].v;
        occ.Bind();

        const float x = vp.l / (float)v.w;
        const float y = vp.b / (float)v.h;
        const float w = vp.w / (float)v.w;
        const float h = vp.h / (float)v.h;

        const OVR::Util::Render::DistortionConfig& Distortion = stereo.GetDistortionConfig();
        const float as = stereo.GetAspect();
        const float scaleFactor = 1.0f / Distortion.Scale;
        occ.SetUniform("LensCenter", x + (w + (1-i*2)*Distortion.XCenterOffset * 0.5f)*0.5f, y + h*0.5f);
        occ.SetUniform("ScreenCenter", x + w*0.5f, y + h*0.5f);
        occ.SetUniform("Scale",   (w/2.0f) * scaleFactor, (h/2.0f) * scaleFactor * as);
        occ.SetUniform("ScaleIn", (2.0f/w),               (2.0f/h) / as);
        occ.SetUniform("HmdWarpParam", HMD.DistortionK[0], HMD.DistortionK[1], HMD.DistortionK[2], HMD.DistortionK[3] );
        occ.SetUniform("ChromAbParam", HMD.ChromaAbCorrection[0], HMD.ChromaAbCorrection[1], HMD.ChromaAbCorrection[2], HMD.ChromaAbCorrection[3]);

        vp.Activate();
        colourbuffer.RenderToViewport(vp, false);

        occ.Unbind();
    }
}

void OculusHud::Render()
{
    if(show) {
        // If render function defined, use it to render left / right images
        // to framebuffer
        if(this->extern_draw_function) {
            framebuffer.Bind();
            glClearColor(1,1,1,0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            for(int i=0; i<2; ++i) {
                eyeview[i].Activate();
                default_cam.ApplyNView(i);
                this->extern_draw_function(*this);
            }
            framebuffer.Unbind();
        }

        RenderFramebuffer();
    }
}

const char* OculusHud::PostProcessFullFragShaderSrc =
    "uniform vec2 LensCenter;\n"
    "uniform vec2 ScreenCenter;\n"
    "uniform vec2 Scale;\n"
    "uniform vec2 ScaleIn;\n"
    "uniform vec4 HmdWarpParam;\n"
    "uniform vec4 ChromAbParam;\n"
    "uniform sampler2D Texture0;\n"
    "\n"
    // Scales input texture coordinates for distortion.
    // ScaleIn maps texture coordinates to Scales to ([-1, 1]), although top/bottom will be
    // larger due to aspect ratio.
    "void main()\n"
    "{\n"
    "   vec2  theta = (gl_TexCoord[0].xy - LensCenter) * ScaleIn;\n" // Scales to [-1, 1]
    "   float rSq= theta.x * theta.x + theta.y * theta.y;\n"
    "   vec2  theta1 = theta * (HmdWarpParam.x + rSq*(HmdWarpParam.y + rSq*(HmdWarpParam.z + rSq*HmdWarpParam.w) ) );\n"
    "   \n"
    "   // Detect whether blue texture coordinates are out of range since these will scaled out the furthest.\n"
    "   vec2 thetaBlue = theta1 * (ChromAbParam.z + ChromAbParam.w * rSq);\n"
    "   vec2 tcBlue = LensCenter + Scale * thetaBlue;\n"
    "   if (!all(equal(clamp(tcBlue, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tcBlue)))\n"
    "   {\n"
    "       gl_FragColor = vec4(0);\n"
    "       return;\n"
    "   }\n"
    "   \n"
    "   // Now do blue texture lookup.\n"
    "   float blue = texture2D(Texture0, tcBlue).b;\n"
    "   \n"
    "   // Do green lookup (no scaling).\n"
    "   vec2  tcGreen = LensCenter + Scale * theta1;\n"
    "   vec4  center = texture2D(Texture0, tcGreen);\n"
    "   \n"
    "   // Do red scale and lookup.\n"
    "   vec2  thetaRed = theta1 * (ChromAbParam.x + ChromAbParam.y * rSq);\n"
    "   vec2  tcRed = LensCenter + Scale * thetaRed;\n"
    "   float red = texture2D(Texture0, tcRed).r;\n"
    "   \n"
    "   gl_FragColor = vec4(red, center.g, blue, 1);\n"
    "}\n";

void OculusHud::InitialiseOculus()
{
    OVR::System::Init();
    pManager = *OVR::DeviceManager::Create();
    if(!pManager) {
        throw std::runtime_error("Unable to create Oculus device manager\n");
    }

    pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
    if (pHMD) {
        pHMD->GetDeviceInfo(&HMD);
        pSensor = *pHMD->GetSensor();
        if (pSensor) {
            pFusionResult = std::unique_ptr<OVR::SensorFusion>(new OVR::SensorFusion());
            pFusionResult->AttachToSensor(pSensor);
        }else{
            pango_print_error("Unable to get sensor\n");
        }
    }else{
        pango_print_error("Unable to create device\n");
        // Set defaults instead
        HMD.HResolution            = 1280;
        HMD.VResolution            = 800;
        HMD.HScreenSize            = 0.14976f;
        HMD.VScreenSize            = HMD.HScreenSize / (1280.0/800.0);
        HMD.InterpupillaryDistance = 0.064f;
        HMD.LensSeparationDistance = 0.0635f;
        HMD.EyeToScreenDistance    = 0.041f;
        HMD.DistortionK[0]         = 1.0f;
        HMD.DistortionK[1]         = 0.22f;
        HMD.DistortionK[2]         = 0.24f;
        HMD.DistortionK[3]         = 0;
    }

    stereo.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
    stereo.SetFullViewport(OVR::Util::Render::Viewport(0,0, HMD.HResolution, HMD.VResolution) );
    stereo.SetHMDInfo(HMD);

    if (HMD.HScreenSize > 0.140f) {
        stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
    }else{
        stereo.SetDistortionFitPointVP(0.0f, 1.0f);
    }
}

void OculusHud::InitialiseFramebuffer()
{
    colourbuffer.Reinitialise(HMD.HResolution, HMD.VResolution, GL_RGBA8);
    depthbuffer.Reinitialise(HMD.HResolution, HMD.VResolution, GL_DEPTH_COMPONENT24);
    framebuffer.AttachDepth(depthbuffer);
    framebuffer.AttachColour(colourbuffer);
}

void OculusHud::InitialiseShader()
{
    occ.AddShader(pangolin::GlSlFragmentShader, PostProcessFullFragShaderSrc);
    occ.Link();
}

Handler3DOculus::Handler3DOculus(OculusHud& oculus, pangolin::OpenGlRenderState& cam_state, pangolin::AxisDirection enforce_up, float trans_scale)
    : pangolin::Handler3D(cam_state,enforce_up, trans_scale), oculus(oculus)
{
}

void Handler3DOculus::GetPosNormal(pangolin::View& view, int x, int y, double p[3], double Pw[3], double Pc[3], double n[3], double default_z)
{
    // TODO: Unwarp coordinates into framebuffer.
    oculus.Framebuffer().Bind();
    Handler3D::GetPosNormal(view,x,y,p,Pw,Pc,n,default_z);
    oculus.Framebuffer().Unbind();
}

}
