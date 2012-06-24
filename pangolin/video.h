/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#ifndef PANGOLIN_VIDEO_H
#define PANGOLIN_VIDEO_H

#include "pangolin.h"

// Pangolin video supports various cameras and file formats through
// different 3rd party libraries.
//
// Video URI's take the following form:
//  scheme:[param1=value1,param2=value2,...]//device
//
// scheme = file | dc1394 | v4l | convert | mjpeg
//
// file/files - read PVN file format (pangolin video) or other formats using ffmpeg
//  e.g. "file:[realtime=1]///home/user/video/movie.pvn"
//  e.g. "file:[stream=1]///home/user/video/movie.avi"
//  e.g. "files:///home/user/seqiemce/foo%03d.jpeg"
//
// dc1394 - capture video through a firewire camera
//  e.g. "dc1394:[fmt=RGB24,size=640x480,fps=30,iso=400,dma=10]//0"
//  e.g. "dc1394:[fmt=FORMAT7_1,size=640x480,pos=2+2,iso=400,dma=10]//0"
//
// v4l - capture video from a Video4Linux (USB) camera (normally YUVY422 format)
//  e.g. "v4l:///dev/video0"
//
// convert - use FFMPEG to convert between video pixel formats
//  e.g. "convert:[fmt=RGB24]//v4l:///dev/video0"
//
// mjpeg - capture from (possibly networked) motion jpeg stream using FFMPEG
//  e.g. "mjpeg://http://127.0.0.1/?action=stream"

namespace pangolin
{
    struct VideoException : std::exception
    {
        VideoException(std::string str) : desc(str) {}
        VideoException(std::string str, std::string detail) {
            desc = str + "\n\t" + detail;
        }
        ~VideoException() throw() {}
        const char* what() const throw() { return desc.c_str(); }
        std::string desc;
    };

    struct VideoPixelFormat
    {
        std::string format;
        unsigned int channels;
        unsigned int channel_bits[4];
        unsigned int bpp;
        bool planar;
    };

    struct Uri
    {
        std::string scheme;
        std::string url;
        std::map<std::string,std::string> params;
    };

    //! Return Pixel Format properties given string specification in
    //! FFMPEG notation.
    VideoPixelFormat VideoFormatFromString(const std::string& format);

    //! Interface to video capture sources
    struct VideoInterface
    {
        virtual ~VideoInterface() {}
        virtual unsigned Width() const = 0;
        virtual unsigned Height() const = 0;
        virtual size_t SizeBytes() const = 0;

        virtual std::string PixFormat() const = 0;

        virtual void Start() = 0;
        virtual void Stop() = 0;

        //! Copy the next frame from the camera to image.
        //! Optionally wait for a frame if one isn't ready
        //! Returns true iff image was copied
        virtual bool GrabNext( unsigned char* image, bool wait = true ) = 0;

        //! Copy the newest frame from the camera to image
        //! discarding all older frames.
        //! Optionally wait for a frame if one isn't ready
        //! Returns true iff image was copied
        virtual bool GrabNewest( unsigned char* image, bool wait = true ) = 0;
    };

    struct VideoInput : public VideoInterface
    {
        VideoInput();
        VideoInput(std::string uri);
        ~VideoInput();

        void Open(std::string uri);
        void Reset();

        unsigned Width() const;
        unsigned Height() const;
        size_t SizeBytes() const;
        std::string PixFormat() const;

        void Start();
        void Stop();
        bool GrabNext( unsigned char* image, bool wait = true );
        bool GrabNewest( unsigned char* image, bool wait = true );

    protected:
        std::string uri;
        VideoInterface* video;
    };

    //! Open Video Interface from string specification (as described in this files header)
    VideoInterface* OpenVideo(std::string uri);

    //! Parse string as Video URI
    Uri ParseUri(std::string str_uri);
}

#endif // PANGOLIN_VIDEO_H
