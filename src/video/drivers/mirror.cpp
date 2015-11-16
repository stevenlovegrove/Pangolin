/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
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

#include <pangolin/video/drivers/mirror.h>

namespace pangolin
{

MirrorVideo::MirrorVideo(VideoInterface* src)
    : videoin(src), size_bytes(0), buffer(0)
{
    if(!src) {
        throw VideoException("MirrorVideo: VideoInterface in must not be null");
    }

    inputs.push_back(videoin);

    streams = src->Streams();
    size_bytes = src->SizeBytes();
    buffer = new unsigned char[size_bytes];
}

MirrorVideo::~MirrorVideo()
{
    delete[] buffer;
    delete videoin;
}

//! Implement VideoInput::Start()
void MirrorVideo::Start()
{
    videoin->Start();
}

//! Implement VideoInput::Stop()
void MirrorVideo::Stop()
{
    videoin->Stop();
}

//! Implement VideoInput::SizeBytes()
size_t MirrorVideo::SizeBytes() const
{
    return size_bytes;
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& MirrorVideo::Streams() const
{
    return streams;
}

//! Implement VideoInput::GrabNext()
bool MirrorVideo::GrabNext( unsigned char* image, bool wait )
{    
    if(videoin->GrabNext(buffer,wait)) {
        for(size_t s=0; s<streams.size(); ++s) {
            Image<unsigned char> img_in  = videoin->Streams()[s].StreamImage(buffer);
            Image<unsigned char> img_out = Streams()[s].StreamImage(image);

            const size_t bytes_pp = Streams()[s].PixFormat().bpp / 8;

            for(size_t y=0; y < img_out.h; ++y) {
                for(size_t x=0; x < img_out.w / 2; ++x) {
                    memcpy(
                        img_out.ptr + y*img_out.pitch + (img_out.w-1-x)*bytes_pp,
                        img_in.ptr  + y*img_in.pitch  + x*bytes_pp,
                        bytes_pp
                    );
                    memcpy(
                        img_out.ptr + y*img_out.pitch + x*bytes_pp,
                        img_in.ptr  + y*img_in.pitch  + (img_in.w-x)*bytes_pp,
                        bytes_pp
                    );
                }
            }
        }
        return true;
    }else{
        return false;
    }
}

//! Implement VideoInput::GrabNewest()
bool MirrorVideo::GrabNewest( unsigned char* image, bool wait )
{
    if(videoin->GrabNewest(buffer,wait)) {
        for(size_t s=0; s<streams.size(); ++s) {
            Image<unsigned char> img_in  = videoin->Streams()[s].StreamImage(buffer);
            Image<unsigned char> img_out = Streams()[s].StreamImage(image);

            const size_t bytes_pp = Streams()[s].PixFormat().bpp / 8;

            for(size_t y=0; y < img_out.h; ++y) {
                for(size_t x=0; x < img_out.w / 2; ++x) {
                    memcpy(
                        img_out.ptr + y*img_out.pitch + (img_out.w-1-x)*bytes_pp,
                        img_in.ptr  + y*img_in.pitch  + x*bytes_pp,
                        bytes_pp
                    );
                    memcpy(
                        img_out.ptr + y*img_out.pitch + x*bytes_pp,
                        img_in.ptr  + y*img_in.pitch  + (img_in.w-x)*bytes_pp,
                        bytes_pp
                    );
                }
            }
        }
        return true;
    }else{
        return false;
    }
}

std::vector<VideoInterface*>& MirrorVideo::InputStreams()
{
    return inputs;
}


}
