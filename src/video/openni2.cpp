/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Richard Newcombe
 *               2014 Steven Lovegrove
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

#include <pangolin/video/openni2.h>

#include <PS1080.h>

namespace pangolin
{

OpenNiVideo2::OpenNiVideo2(OpenNiSensorType s1, OpenNiSensorType s2, ImageDim dim, int fps)
{
    fromFile = false;

    sensor_type[0] = s1;
    sensor_type[1] = s2;

    const char* deviceURI = openni::ANY_DEVICE;

    rc = openni::OpenNI::initialize();
    if (rc != openni::STATUS_OK) {
        pango_print_error("Unable to initialise OpenNI library: %s\n", openni::OpenNI::getExtendedError());
        throw VideoException("Unable to initialise OpenNI library");
    }

    if(deviceURI!=NULL){
        fromFile = true;
    }else{
        fromFile = false;
    }

    rc = device.open(deviceURI);
    if (rc != openni::STATUS_OK) {
        pango_print_error("Failed to open device: %s\n", openni::OpenNI::getExtendedError());
        throw VideoException("Failed to open device");
    }

    use_depth = false;
    use_ir = false;
    use_rgb = false;
    depth_to_color = false;
    use_ir_and_rgb = false;

    int useModeRGB=19;
    int useDepth=5;

    for(int i=0; i<2; ++i) {
        VideoPixelFormat fmt;

        openni::VideoMode m_Format;
        m_Format.setFps(fps);
        m_Format.setResolution(dim.x, dim.y);

        // Establish output pixel format for sensor streams
        switch( sensor_type[i] ) {
        case OpenNiDepth:
            fmt = VideoFormatFromString("GRAY16LE");
        case OpenNiDepthRegistered:
        case OpenNiIr:
        case OpenNiIrProj:
        case OpenNiGrey:
            fmt = VideoFormatFromString("GRAY8");
            break;
        case OpenNiIr8bit:
            fmt = VideoFormatFromString("GRAY8");
            break;
        case OpenNiIr8bitProj:
            fmt = VideoFormatFromString("GRAY8");
            break;
        case OpenNiRgb:
            fmt = VideoFormatFromString("RGB24");
            break;
        case OpenNiUnassigned:
        default:
            continue;
        }

        switch( sensor_type[i] ) {
        case OpenNiDepthRegistered:
            depth_to_color = true;
        case OpenNiDepth:
        {
            use_depth = true;
            const openni::Array<openni::VideoMode>& modes = device.getSensorInfo(openni::SENSOR_DEPTH)->getSupportedVideoModes();
            m_Format = modes[useDepth];
            break;
        }
        case OpenNiIr:
        case OpenNiIr8bit:
            use_ir = true;
            break;
        case OpenNiGrey:
            use_rgb = true;
            break;
        case OpenNiRgb:
        {
            const openni::Array<openni::VideoMode>& modes = device.getSensorInfo(openni::SENSOR_COLOR)->getSupportedVideoModes();
            m_Format = modes[useModeRGB];
            use_rgb = true;
            break;
        }
        case OpenNiIrProj:
        case OpenNiIr8bitProj:
        case OpenNiUnassigned:
            break;
        }

        const StreamInfo stream(
            fmt, m_Format.getResolutionX(), m_Format.getResolutionY(),
            (m_Format.getResolutionX() * fmt.bpp) / 8,
            (unsigned char*)0 + sizeBytes
        );

        sizeBytes += stream.SizeBytes();
        streams.push_back(stream);
    }
    use_ir_and_rgb = use_rgb && use_ir;


    if(use_ir)
    {
        rc = ir_ps.create(device, openni::SENSOR_IR);
        if (rc == openni::STATUS_OK)
        {
            const openni::Array<openni::VideoMode>& modes = device.getSensorInfo(openni::SENSOR_IR)->getSupportedVideoModes();
            for(int i  = 0; i < modes.getSize();i++){
                std::cout << i << " SENSOR_IR Mode " << modes[i].getResolutionX() <<" " <<
                             modes[i].getFps() << " " <<
                             modes[i].getPixelFormat() << std::endl;
            }
            rc = ir_ps.setVideoMode(modes[5]);
            ir_ps.setMirroringEnabled(false);
            //color.getVideoMode().setResolution(this->cam.imageSize.x,this->cam.imageSize.y);
            if(!use_ir_and_rgb) rc = ir_ps.start();

            if (rc != openni::STATUS_OK)
            {
                printf("SimpleViewer: Couldn't start IR stream:\n%s\n", openni::OpenNI::getExtendedError());
                ir_ps.destroy();
            }else{
                //rc =   ir_ps.start();
                if (rc != openni::STATUS_OK)
                {
                    printf("Couldn't start the IR stream\n%s\n", openni::OpenNI::getExtendedError());
                    //return 4;
                }
            }
        }
        else
        {
            printf("SimpleViewer: Couldn't find IR stream:\n%s\n", openni::OpenNI::getExtendedError());
        }
    }

    if(use_depth){
        rc = depth_ps.create(device, openni::SENSOR_DEPTH);

        if (rc == openni::STATUS_OK)
        {
            const openni::Array<openni::VideoMode>& modes = device.getSensorInfo(openni::SENSOR_DEPTH)->getSupportedVideoModes();
            for(int i  = 0; i < modes.getSize();i++){
                std::cout << i << " Depth Mode " << modes[i].getResolutionX() <<" " <<
                             modes[i].getFps() << " " <<
                             modes[i].getPixelFormat() << std::endl;
            }
            if(!fromFile){
                rc = depth_ps.setVideoMode(modes[5]);
            }


            bool bCloseRange=false;
            bool bHoleFiler =false;
            depth_ps.setProperty(XN_STREAM_PROPERTY_CLOSE_RANGE, bCloseRange);
            depth_ps.setProperty(XN_STREAM_PROPERTY_HOLE_FILTER,bHoleFiler);
            depth_ps.setProperty(XN_STREAM_PROPERTY_GAIN,50);

            if (rc != openni::STATUS_OK)
            {
                std::cout << "could not set sensor" <<std::endl;
            }
            depth_ps.setMirroringEnabled(false);
            rc = depth_ps.start();
            if (rc != openni::STATUS_OK)
            {
                pango_print_error("Couldn't start depth_ps stream:\n%s\n", openni::OpenNI::getExtendedError());
                depth_ps.destroy();
            }
        }
        else
        {
            pango_print_error("Couldn't find depth_ps stream:\n%s\n", openni::OpenNI::getExtendedError());
        }
    }


    if(use_rgb)
    {
        rc = color.create(device, openni::SENSOR_COLOR);
        if (rc == openni::STATUS_OK)
        {
            //color.getVideoMode().setResolution(this->cam.imageSize.x,this->cam.imageSize.y);
            const openni::Array<openni::VideoMode>& modes = device.getSensorInfo(openni::SENSOR_COLOR)->getSupportedVideoModes();

            for(int i  = 0; i < modes.getSize();i++){
                std::cout << i << " Colour Mode " << modes[i].getResolutionX() <<" " << modes[i].getResolutionY() << " " <<
                             modes[i].getFps() << " " <<
                             modes[i].getPixelFormat() << std::endl;


            }

            //                rc = color.start();
            if(!fromFile){

                std::cout << "using mode " << useModeRGB << std::endl;
                std::cout  << " Colour Mode " << modes[useModeRGB].getResolutionX() <<" " <<
                              modes[useModeRGB].getFps() << " " <<
                              modes[useModeRGB].getPixelFormat() << std::endl;
                rc = color.setVideoMode(modes[useModeRGB]);
            }


            color.getCameraSettings()->setAutoExposureEnabled(false);
            color.getCameraSettings()->setAutoWhiteBalanceEnabled(false);

            color.setMirroringEnabled(false);
            if(!use_ir_and_rgb) rc = color.start();

            //            if(color.getVideoMode().getResolutionX()!=depth_ps.getVideoMode().getResolutionX()){

            //                color.stop();
            //                color.destroy();
            //            }

            if (rc != openni::STATUS_OK)
            {
                printf("SimpleViewer: Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
                color.destroy();
            }
        }
        else
        {
            printf("SimpleViewer: Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
        }
    }


    /*  if (!depth_ps.isValid() ) // || (withColour && !color.isValid() ) || (withIR && !ir_ps.isValid()) )
    {
        printf("SimpleViewer: No valid streams. Exiting\n");
        openni::OpenNI::shutdown();
        //return 2;
    }*/

    if(fromFile) device.getPlaybackControl()->setSpeed(-1);


    if (rc != openni::STATUS_OK)
    {
        printf("Couldn't start the depth stream\n%s\n", openni::OpenNI::getExtendedError());
        //return 4;
    }

    if(use_rgb)
    {
        openni::Status rc = device.setDepthColorSyncEnabled(false);

        std::cout << "primsense: colour/depth sync is not enabled "    <<std::endl;


        //        rc = device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
        //        if(rc!=openni::STATUS_OK){
        //            std::cout << "primsense: colour/depth reg is not enabled "    <<std::endl;
        //        }else{
        //            std::cout << "primsense: colour/depth reg is enabled "    <<std::endl;
        //        }


        //            focal_length = 525;
        //device.s  etImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
        //focal_length = 575;
    }else{
        device.setDepthColorSyncEnabled(false);
        device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
        //            focal_length =  575;
        //             focal_length = 525;
    }

    //set the focal length depending on whether we are in registration mode or not.

    if(device.getImageRegistrationMode()==openni::IMAGE_REGISTRATION_OFF){
        //focal_length = 575;
        //focal_length = 565;
    }else{
        //focal_length = 550;
    }
    //        focal_length = 575;

    Start();
}

OpenNiVideo2::~OpenNiVideo2()
{

    depth_ps.stop();
    depth_ps.destroy();
    color.stop();
    color.destroy();
    openni::OpenNI::shutdown();
}

size_t OpenNiVideo2::SizeBytes() const
{
    return sizeBytes;
}

const std::vector<StreamInfo>& OpenNiVideo2::Streams() const
{
    return streams;
}

void OpenNiVideo2::Start()
{
}

void OpenNiVideo2::Stop()
{
}

bool OpenNiVideo2::GrabNext( unsigned char* image, bool wait )
{
    if (rc!= openni::STATUS_OK) {
        pango_print_error("Failed updating data");
        return false;
    }else{
        unsigned char* out_img = image;

        for(int i=0; i<2; ++i) {
            switch (sensor_type[i]) {
            case OpenNiDepth:
            {
                rc = depth_ps.readFrame(&depth_frame);
                openni::DepthPixel* pDepth = (openni::DepthPixel*)depth_frame.getData();
                memcpy(out_img,pDepth, streams[i].SizeBytes());
                break;
            }
            case OpenNiDepthRegistered:
            {
                rc = depth_ps.readFrame(&depth_frame);
                openni::DepthPixel* pDepth = (openni::DepthPixel*)depth_frame.getData();
                break;
            }
            case OpenNiIr:
            case OpenNiIrProj:
                throw std::runtime_error("Not implemented");

            case OpenNiIr8bit:
            case OpenNiIr8bitProj:
            {
                if(use_ir_and_rgb){
                    ir_ps.start();
                }

                const int w = streams[i].Width();
                const int h = streams[i].Height();

                rc = ir_ps.readFrame(&m_IRFrame);
                openni::RGB888Pixel* pColour = (openni::RGB888Pixel*)m_IRFrame.getData();
                for(int i = 0 ; i  < w*h;i++){
                    openni::RGB888Pixel rgb = pColour[i];
                    int grey = ((int)(rgb.r&0xFF) +  (int)(rgb.g&0xFF) + (int)(rgb.b&0xFF))/3;
                    grey = std::min(255,std::max(0,grey));
                    out_img[i] = grey;
                }

                if(use_ir_and_rgb){
                    ir_ps.stop();
                }

                break;
            }

            case OpenNiRgb:
            {
                int frame_collect = 1;
                if(use_ir_and_rgb){
                    frame_collect =2;
                    color.start();
                }

                for(int n = 0 ; n < frame_collect;n++){
                    rc = color.readFrame(&m_colorFrame);
                }

                openni::RGB888Pixel* pColour = (openni::RGB888Pixel*)m_colorFrame.getData();

                //m_colorFrame.getHeight()
                //memcpy(out_img,pColour, streams[i].SizeBytes());
                std::cout << "rgb sizebytes " << streams[i].SizeBytes() << std::endl;
                memcpy(out_img,pColour, m_colorFrame.getStrideInBytes()*m_colorFrame.getHeight());// streams[i].SizeBytes());

                if(use_ir_and_rgb){
                    color.stop();
                }

                break;
            }
            case OpenNiGrey:
            {
                const int w = streams[i].Width();
                const int h = streams[i].Height();

                int frame_collect = 1;
                if(use_ir_and_rgb){
                    frame_collect =2;
                    color.start();
                }
                for(int n = 0 ; n < frame_collect;n++){
                    rc = color.readFrame(&m_colorFrame);
                }
                openni::RGB888Pixel* pColour = (openni::RGB888Pixel*)m_colorFrame.getData();
                for(int i = 0 ; i  <w*h;i++){
                    openni::RGB888Pixel rgb = pColour[i];
                    int grey = ((int)(rgb.r&0xFF) +  (int)(rgb.g&0xFF) + (int)(rgb.b&0xFF))/3;
                    grey = std::min(255,std::max(0,grey));
                    out_img[i] = grey;
                }

                if(use_ir_and_rgb){
                    color.stop();
                }

                break;
            }

            case OpenNiUnassigned:
                break;
            }

            out_img += streams[i].SizeBytes();
        }

        return true;
    }
}

bool OpenNiVideo2::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image,wait);
}

}
