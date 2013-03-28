/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/video/openni.h>

namespace pangolin
{

OpenNiVideo::OpenNiVideo(OpenNiSensorType s1, OpenNiSensorType s2)
    :s1(s1), s2(s2)
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
    
    switch(s1) {
    case OpenNiDepth:
    case OpenNiIr:
        s1SizeBytes = XN_VGA_X_RES * XN_VGA_Y_RES * sizeof(XnDepthPixel);
        break;
    case OpenNiRgb:
        s1SizeBytes = XN_VGA_X_RES * XN_VGA_Y_RES * sizeof(XnUInt8) * 3;
        break;
    case OpenNiUnassigned:
    default:
        s1SizeBytes = 0;
    }
    
    switch(s2) {
    case OpenNiDepth:
    case OpenNiIr:
        s2SizeBytes = XN_VGA_X_RES * XN_VGA_Y_RES * sizeof(XnDepthPixel);
        break;
    case OpenNiRgb:
        s1SizeBytes = XN_VGA_X_RES * XN_VGA_Y_RES * sizeof(XnUInt8) * 3;
        break;
    case OpenNiUnassigned:
    default:
        s2SizeBytes = 0;
    }
    
    sizeBytes = s1SizeBytes + s2SizeBytes;
    
    if( s1 == OpenNiDepth || s2 == OpenNiDepth ) {
        nRetVal = depthNode.Create(context);
        if (nRetVal != XN_STATUS_OK) {
            std::cerr << "depthNode.Create: " << xnGetStatusString(nRetVal) << std::endl;
        }else{
            nRetVal = depthNode.SetMapOutputMode(mapMode);
            if (nRetVal != XN_STATUS_OK) {
                std::cerr << "depthNode.SetMapOutputMode: " << xnGetStatusString(nRetVal) << std::endl;
            }
        }
    }
    
    if( s1 == OpenNiIr || s2 == OpenNiIr ) {
        nRetVal = irNode.Create(context);
        if (nRetVal != XN_STATUS_OK) {
            std::cerr << "irNode.Create: " << xnGetStatusString(nRetVal) << std::endl;
        }else{
            nRetVal = irNode.SetMapOutputMode(mapMode);
            if (nRetVal != XN_STATUS_OK) {
                std::cerr << "irNode.SetMapOutputMode: " << xnGetStatusString(nRetVal) << std::endl;
            }
        }
    }
    
    if( s1 == OpenNiRgb || s2 == OpenNiRgb ) {
        nRetVal = imageNode.Create(context);
        if (nRetVal != XN_STATUS_OK) {
            std::cerr << "imageNode.Create: " << xnGetStatusString(nRetVal) << std::endl;
        }else{
            nRetVal = imageNode.SetMapOutputMode(mapMode);
            if (nRetVal != XN_STATUS_OK) {
                std::cerr << "imageNode.SetMapOutputMode: " << xnGetStatusString(nRetVal) << std::endl;
            }
        }
    }
    
    Start();
}

OpenNiVideo::~OpenNiVideo()
{
    context.Release();
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
    return sizeBytes;
}

VideoPixelFormat OpenNiVideo::PixFormat() const
{
    if(s1 == OpenNiRgb) {
        return VideoFormatFromString("RGB24");
    }else{
        return VideoFormatFromString("GRAY16LE");
    }
}

void OpenNiVideo::Start()
{
    //    XnStatus nRetVal = 
    context.StartGeneratingAll();
}

void OpenNiVideo::Stop()
{
    context.StopGeneratingAll();
}

bool OpenNiVideo::GrabNext( unsigned char* image, bool wait )
{
    //    XnStatus nRetVal = context.WaitAndUpdateAll();
    XnStatus nRetVal = context.WaitAnyUpdateAll();
    //    nRetVal = context.WaitOneUpdateAll(imageNode);
    
    if (nRetVal != XN_STATUS_OK) {
        std::cerr << "Failed updating data: " << xnGetStatusString(nRetVal) << std::endl;
        return false;
    }else{
        if(s1==OpenNiDepth) {
            const XnDepthPixel* pDepthMap = depthNode.GetDepthMap();
            memcpy(image,pDepthMap, s1SizeBytes );
        }else if(s1==OpenNiIr) {
            const XnIRPixel* pIrMap = irNode.GetIRMap();
            memcpy(image,pIrMap, s1SizeBytes);
        }else if(s1==OpenNiRgb) {
            const XnUInt8* pImageMap = imageNode.GetImageMap();
            memcpy(image,pImageMap, s1SizeBytes);
        }
        
        if(s2==OpenNiDepth) {
            const XnDepthPixel* pDepthMap = depthNode.GetDepthMap();
            memcpy(image+s1SizeBytes,pDepthMap, s2SizeBytes);
        }else if(s2==OpenNiIr) {
            const XnIRPixel* pIrMap = irNode.GetIRMap();
            memcpy(image+s1SizeBytes,pIrMap, s2SizeBytes);
        }else if(s2==OpenNiRgb) {
            const XnUInt8* pImageMap = imageNode.GetImageMap();
            memcpy(image+s1SizeBytes,pImageMap, s2SizeBytes);
        }
        return true;
    }
}

bool OpenNiVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image,wait);
}

}

