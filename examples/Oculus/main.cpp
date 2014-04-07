#include <pangolin/pangolin.h>
#include <pangolin/handler_glbuffer.h>
#include <pangolin/glsl.h>

#include <OVR.h>

// Shader with lens distortion and chromatic aberration correction.
static const char* PostProcessFullFragShaderSrc =
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

int main(int argc, char ** argv) {
    pangolin::CreateWindowAndBind("Main",640,480);
    glEnable(GL_DEPTH_TEST);

    OVR::System::Init();

    OVR::Ptr<OVR::DeviceManager> pManager = *OVR::DeviceManager::Create();
    if(!pManager) {
        pango_print_error("Unable to create device manager\n");
        return 0;
    }

    OVR::Ptr<OVR::HMDDevice> pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
    OVR::HMDInfo HMD;
    OVR::Ptr<OVR::SensorDevice> pSensor;
    OVR::SensorFusion FusionResult;

    if (pHMD) {
        pHMD->GetDeviceInfo(&HMD);
        pSensor = *pHMD->GetSensor();
        if (pSensor) {
            FusionResult.AttachToSensor(pSensor);
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

    pangolin::GlTexture colourbuffer(HMD.HResolution, HMD.VResolution, GL_RGBA8);
    pangolin::GlRenderBuffer depthbuffer(HMD.HResolution, HMD.VResolution, GL_DEPTH_COMPONENT24);
    pangolin::GlFramebuffer framebuffer(colourbuffer, depthbuffer);

    pangolin::OpenGlRenderState s_cam;
    s_cam.SetModelViewMatrix( pangolin::ModelViewLookAt(-1.5,1.5,-1.5, 0,0,0, pangolin::AxisY) );

    pangolin::View& FullView = pangolin::DisplayBase();

    pangolin::GlSlProgram occ;
    occ.AddShader(pangolin::GlSlFragmentShader, PostProcessFullFragShaderSrc);
    occ.Link();

    while( !pangolin::ShouldQuit() )
    {
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const OVR::Quatf q = FusionResult.GetOrientation();
        const OVR::Matrix4f T_co = OVR::Matrix4f(q);

        OVR::Util::Render::StereoConfig stereo(
            OVR::Util::Render::Stereo_LeftRight_Multipass,
            OVR::Util::Render::Viewport(0,0, FullView.v.w, FullView.v.h)
        );
        stereo.SetHMDInfo(HMD);
        if (HMD.HScreenSize > 0.140f) {
            stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
        }else{
            stereo.SetDistortionFitPointVP(0.0f, 1.0f);
        }

        framebuffer.Bind();
        glClearColor(1,1,1,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for(int i=0; i<2; ++i) {
            OVR::Util::Render::StereoEyeParams eye =
                    stereo.GetEyeRenderParams( OVR::Util::Render::StereoEye(OVR::Util::Render::StereoEye_Left+i) );

            pangolin::OpenGlRenderState cam( eye.Projection,
                pangolin::OpenGlMatrix(T_co.Inverted() * eye.ViewAdjust) * s_cam.GetModelViewMatrix()
            );

            glViewport(eye.VP.x,eye.VP.y,eye.VP.w,eye.VP.h);
            cam.Apply();
            pangolin::glDrawColouredCube();
        }
        framebuffer.Unbind();

        FullView.Activate();
        glColor3f(1.0,1.0,1.0);

        for(int i=0; i<2; ++i)
        {
            OVR::Util::Render::StereoEyeParams eye =
                    stereo.GetEyeRenderParams( OVR::Util::Render::StereoEye(1+i) );

            pangolin::Viewport vp(eye.VP.x,eye.VP.y,eye.VP.w,eye.VP.h);
            occ.Bind();

            const float x = vp.l / (float)FullView.v.w;
            const float y = vp.b / (float)FullView.v.h;
            const float w = vp.w / (float)FullView.v.w;
            const float h = vp.h / (float)FullView.v.h;

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


        pangolin::FinishFrame();
    }

    return 0;
}
