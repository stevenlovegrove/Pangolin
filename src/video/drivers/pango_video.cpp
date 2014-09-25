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

#include <pangolin/video/drivers/pango_video.h>

#include <pangolin/compat/bind.h>

namespace pangolin
{

PangoVideo::PangoVideo(const std::string& filename, bool realtime)
    : reader(filename, realtime)
{
    src_id = FindSource();

    if(src_id == -1) {
        throw pangolin::VideoException("No appropriate video streams found in log.");
    }
}

PangoVideo::~PangoVideo()
{
}

size_t PangoVideo::SizeBytes() const
{
    return size_bytes;
}

const std::vector<StreamInfo>& PangoVideo::Streams() const
{
    return streams;
}

void PangoVideo::Start()
{

}

void PangoVideo::Stop()
{

}

bool PangoVideo::GrabNext( unsigned char* image, bool wait )
{
    if(reader.ReadToSourceFrameAndLock(src_id)) {
        // read this frames actual data
        reader.Read((char*)image, size_bytes);
        reader.ReleaseSourceFrameLock(src_id);
        return true;
    }else{
        return false;
    }
}

bool PangoVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image, wait);
}

int PangoVideo::FindSource()
{
    for(PacketStreamSourceId src_id=0; src_id < reader.Sources().size(); ++src_id)
    {
        const PacketStreamSource& src = reader.Sources()[src_id];

        try {
            if( src.type.compare("raw_video") ) {
                // Read sources header
                size_bytes = 0;

                const picojson::value& json_streams = src.header["streams"];
                const size_t num_streams = json_streams.size();
                for(size_t i=0; i<num_streams; ++i) {
                    const picojson::value& json_stream = json_streams[i];
                    StreamInfo si(
                        VideoFormatFromString(
                            json_stream["encoding"].get<std::string>()
                        ),
                        json_stream["width"].get<int64_t>(),
                        json_stream["height"].get<int64_t>(),
                        json_stream["pitch"].get<int64_t>(),
                        (unsigned char*)0 + json_stream["offset"].get<int64_t>()
                    );

                    size_bytes += si.SizeBytes();
                    streams.push_back(si);
                }

                return src_id;
            }
        }catch(...) {
            pango_print_info("Unable to parse PacketStream Source. File version incompatible.\n");
        }
    }

    return -1;
}

}
