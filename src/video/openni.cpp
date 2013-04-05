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
{
    sensor_type[0] = s1;
    sensor_type[1] = s2;
    
    XnStatus nRetVal = XN_STATUS_OK;
    nRetVal = context.Init();
    if (nRetVal != XN_STATUS_OK) {
        std::cerr << "context.Init: " << xnGetStatusString(nRetVal) << std::endl;
    }
    
    XnMapOutputMode mapMode;
    mapMode.nXRes = XN_VGA_X_RES;
    mapMode.nYRes = XN_VGA_Y_RES;
    mapMode.nFPS = 30;
    
    sizeBytes = 0;
    
    for(int i=0; i<2; ++i) {
        VideoPixelFormat fmt;
        
        switch( sensor_type[i] ) {
        case OpenNiDepth:
        case OpenNiIr:
            sensor_SizeBytes[i] = XN_VGA_X_RES * XN_VGA_Y_RES * sizeof(XnDepthPixel);
            fmt = VideoFormatFromString("GRAY16LE");
            break;
        case OpenNiRgb:
            fmt = VideoFormatFromString("RGB24");
            sensor_SizeBytes[i] = XN_VGA_X_RES * XN_VGA_Y_RES * sizeof(XnUInt8) * 3;
            break;
        case OpenNiUnassigned:
        default:
            sensor_SizeBytes[i] = 0;
            continue;
        }
        
        const StreamInfo stream(fmt, XN_VGA_X_RES, XN_VGA_Y_RES, (XN_VGA_X_RES * fmt.bpp) / 8, 0);
        streams.push_back(stream);
        sizeBytes += sensor_SizeBytes[i];
    }
    
    if( sensor_type[0] == OpenNiDepth || sensor_type[1] == OpenNiDepth ) {
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
    
    if( sensor_type[0] == OpenNiIr || sensor_type[1] == OpenNiIr ) {
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
    
    if( sensor_type[0] == OpenNiRgb || sensor_type[1] == OpenNiRgb ) {
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

size_t OpenNiVideo::SizeBytes() const
{
    return sizeBytes;
}

const std::vector<StreamInfo>& OpenNiVideo::Streams() const
{
    return streams;
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
        unsigned char* out_img = image;
        
        for(int i=0; i<2; ++i) {
            if(sensor_type[i]==OpenNiDepth) {
                const XnDepthPixel* pDepthMap = depthNode.GetDepthMap();
                memcpy(out_img,pDepthMap, sensor_SizeBytes[i] );
            }else if(sensor_type[i]==OpenNiIr) {
                const XnIRPixel* pIrMap = irNode.GetIRMap();
                memcpy(out_img,pIrMap, sensor_SizeBytes[i]);
            }else if(sensor_type[i]==OpenNiRgb) {
                const XnUInt8* pImageMap = imageNode.GetImageMap();
                memcpy(out_img,pImageMap, sensor_SizeBytes[i]);
            }
            out_img += sensor_SizeBytes[i];
        }
        
        return true;
    }
}

bool OpenNiVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image,wait);
}

}

