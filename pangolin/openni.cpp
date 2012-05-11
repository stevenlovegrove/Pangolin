#include "openni.h"

#ifdef HAVE_OPENNI

namespace pangolin
{

OpenNiVideo::OpenNiVideo()
{
    XnStatus nRetVal = XN_STATUS_OK;
    nRetVal = context.Init();
    if (nRetVal != XN_STATUS_OK) {
        std::cerr << "context.Init: " << xnGetStatusString(nRetVal) << std::endl;
    }

    XnMapOutputMode mapMode;
    mapMode.nXRes = XN_VGA_X_RES;
    mapMode.nYRes = XN_VGA_Y_RES;
    mapMode.nFPS = 30;

    nRetVal = depthNode.Create(context);
    if (nRetVal != XN_STATUS_OK) {
        std::cerr << "depthNode.Create: " << xnGetStatusString(nRetVal) << std::endl;
    }else{
        nRetVal = depthNode.SetMapOutputMode(mapMode);
        if (nRetVal != XN_STATUS_OK) {
            std::cerr << "depthNode.SetMapOutputMode: " << xnGetStatusString(nRetVal) << std::endl;
        }
    }

    nRetVal = imageNode.Create(context);
    if (nRetVal != XN_STATUS_OK) {
        std::cerr << "imageNode.Create: " << xnGetStatusString(nRetVal) << std::endl;
    }else{
        nRetVal = imageNode.SetMapOutputMode(mapMode);
        if (nRetVal != XN_STATUS_OK) {
            std::cerr << "imageNode.SetMapOutputMode: " << xnGetStatusString(nRetVal) << std::endl;
        }
    }

    Start();
}

OpenNiVideo::~OpenNiVideo()
{
    context.Shutdown();
}

unsigned OpenNiVideo::Width() const
{
    return XN_VGA_X_RES;
}

unsigned OpenNiVideo::Height() const
{
    return XN_VGA_Y_RES;
}

size_t OpenNiVideo::SizeBytes() const
{
//    return XN_VGA_X_RES * XN_VGA_Y_RES * sizeof(XnDepthPixel);
    return XN_VGA_X_RES * XN_VGA_Y_RES * sizeof(XnUInt8) * 3;
}

std::string OpenNiVideo::PixFormat() const
{
    return "RGB24";
//    return "GRAY16LE";
}

void OpenNiVideo::Start()
{
    XnStatus nRetVal = context.StartGeneratingAll();
}

void OpenNiVideo::Stop()
{
    context.StopGeneratingAll();
}

bool OpenNiVideo::GrabNext( unsigned char* image, bool wait )
{
//    m_Context.WaitAnyUpdateAll();

    XnStatus nRetVal = context.WaitOneUpdateAll(depthNode);
    if (nRetVal != XN_STATUS_OK) {
        std::cerr << "Failed updating data: " << xnGetStatusString(nRetVal) << std::endl;
    }else{
        const XnDepthPixel* pDepthMap = depthNode.GetDepthMap();
//        memcpy(image,pDepthMap,SizeBytes());
    }

    nRetVal = context.WaitOneUpdateAll(imageNode);
    if (nRetVal != XN_STATUS_OK) {
        std::cerr << "Failed updating data: " << xnGetStatusString(nRetVal) << std::endl;
    }else{
        const XnUInt8* pImageMap = imageNode.GetImageMap();
        memcpy(image,pImageMap,SizeBytes());
    }

}

bool OpenNiVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image,wait);
}

}

#endif // HAVE_OPENNI

