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
    //"varying vec2 oTexCoord;\n"
    "\n"
    // Scales input texture coordinates for distortion.
    // ScaleIn maps texture coordinates to Scales to ([-1, 1]), although top/bottom will be
    // larger due to aspect ratio.
    "void main()\n"
    "{\n"
    "   vec2  theta = (gl_TexCoord[0].xy - LensCenter) * ScaleIn;\n" // Scales to [-1, 1]
    "   float rSq= theta.x * theta.x + theta.y * theta.y;\n"
    "   vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + "
    "                  HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
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

    const int w = 640;
    const int h = 480;

    // Frame buffer for entire screen
    const unsigned int MAXW = 1440;
    const unsigned int MAXH = 900;
    pangolin::GlTexture colourbuffer(MAXW,MAXH,GL_RGBA8);
    pangolin::GlRenderBuffer depthbuffer(MAXW,MAXH,GL_DEPTH_COMPONENT24);
    pangolin::GlFramebuffer framebuffer(colourbuffer, depthbuffer);

    pangolin::OpenGlRenderState s_cam[] = {
        { pangolin::ProjectionMatrix(w,h,420,420,w/2,h/2,0.2,100),
          pangolin::ModelViewLookAt(-2,2,-2, 0,0,0, pangolin::AxisY) },
        { pangolin::ProjectionMatrix(w,h,420,420,w/2,h/2,0.2,100),
          pangolin::ModelViewLookAt(-2,2,-2, 0,0,0, pangolin::AxisY) }
    };
    pangolin::Handler3DFramebuffer handler[] = {
        {framebuffer, s_cam[0]},
        {framebuffer, s_cam[1]},
    };

    pangolin::View& container = pangolin::DisplayBase();
    pangolin::View view[2];
    for(int i=0; i<2; ++i) {
        container.AddDisplay(view[i]);
        view[i].SetHandler(&handler[i]);
        view[i].SetBounds(0.0,1.0,0.5*i,0.5*i+0.5, -640.0f/480.0f);
    }

    OVR::System::Init();

    OVR::Ptr<OVR::DeviceManager> pManager = *OVR::DeviceManager::Create();
    if(!pManager) {
        pango_print_error("Unable to create device manager\n");
        return 0;
    }

    OVR::Ptr<OVR::HMDDevice> pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
    OVR::HMDInfo hmdInfo;
    OVR::Ptr<OVR::SensorDevice> pSensor;
    OVR::SensorFusion FusionResult;

    if (pHMD) {
        pHMD->GetDeviceInfo(&hmdInfo);
        pSensor = *pHMD->GetSensor();
        if (pSensor) {
            FusionResult.AttachToSensor(pSensor);
        }else{
            pango_print_error("Unable to get sensor\n");
            return 0;
        }
    }else{
        pango_print_error("Unable to create device\n");
        return 0;
    }


    pangolin::GlSlProgram occ;
    occ.AddShader(pangolin::GlSlFragmentShader, PostProcessFullFragShaderSrc);
    occ.Link();

    while( !pangolin::ShouldQuit() )
    {
        const OVR::Quatf q = FusionResult.GetOrientation();
        const OVR::Matrix4f oT_co = OVR::Matrix4f(q).Transposed();
        pangolin::OpenGlMatrix T_co;
        std::copy(&oT_co.M[0][0], &oT_co.M[0][0]+16, T_co.m);
        T_co.Inverse();

        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        framebuffer.Bind();
        glClearColor(1,1,1,0);
        for(int i=0; i<1; ++i) {
            pangolin::OpenGlRenderState cam = s_cam[0];
            pangolin::OpenGlMatrix T_ow = T_co.Inverse() * cam.GetModelViewMatrix();
            cam.GetModelViewMatrix() = T_ow;

            view[i].ActivateScissorAndClear(cam);
            pangolin::glDrawColouredCube();
        }
        framebuffer.Unbind();

        container.ActivateAndScissor();
        glColor3f(1.0,1.0,1.0);

        {
            const int i = 0;
            const pangolin::Viewport vp = view[i].GetBounds();
            const float w = float(vp.w) / float(container.v.w);
            const float h = float(vp.h) / float(container.v.h);
            const float x = float(vp.l) / float(container.v.w);
            const float y = float(vp.b) / float(container.v.h);
            const float as = float(vp.w) / float(vp.h);

            const float lensOffset = hmdInfo.LensSeparationDistance * 0.5f;
            const float lensShift  = hmdInfo.HScreenSize * 0.25f - lensOffset;
            const float lensViewportShift = 4.0f * lensShift / hmdInfo.HScreenSize;
            const float XCenterOffset= lensViewportShift;
            const float Scale = 1.0;
            const float scaleFactor = 1.0f / Scale;

            occ.Bind();

            occ.SetUniform("LensCenter",   x + (w + XCenterOffset * 0.5f)*0.5f, y + h*0.5f);
            occ.SetUniform("ScreenCenter", x + w*0.5f, y + h*0.5f);
            occ.SetUniform("Scale",   (w/2.0f) * scaleFactor, (h/2.0f) * scaleFactor * as);
            occ.SetUniform("ScaleIn", (2.0f/w),               (2.0f/h) / as);
            occ.SetUniform("HmdWarpParam", hmdInfo.DistortionK[0], hmdInfo.DistortionK[1], hmdInfo.DistortionK[2], hmdInfo.DistortionK[3] );
            occ.SetUniform("ChromAbParam", hmdInfo.ChromaAbCorrection[0], hmdInfo.ChromaAbCorrection[1], hmdInfo.ChromaAbCorrection[2], hmdInfo.ChromaAbCorrection[3]);

            colourbuffer.Render(container.v, false);

            occ.Unbind();
        }


        pangolin::FinishFrame();
    }

    return 0;
}
