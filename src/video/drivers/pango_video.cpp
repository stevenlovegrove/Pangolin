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

PangoVideo::PangoVideo(const std::string& filename)
    : reader(filename)
{
    reader.RegisterSourceHeaderHandler(*this);
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
    if(reader.ProcessUpToNextSourceFrame(src_id)) {
        // read this frames actual data
        reader.Read((char*)image, size_bytes);
        reader.ReadTag();
        return true;
    }else{
        return false;
    }
}

bool PangoVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image, wait);
}

void PangoVideo::NewSource(PacketStreamSourceId src_id, const PacketStreamSource& src)
{
    std::cout << "New Stream Source: " << src_id << std::endl;

    if( src.type.compare("raw_video") ) {
        // Register to keep frames of this type for processing
        reader.RegisterFrameHandler(src_id);
        this->src_id = src_id;

        // Read sources header
        size_bytes = 0;

        if(src.header.is<picojson::object>()) {
            const picojson::value& json_streams = src.header.get("streams");
            if(json_streams.is<picojson::array>() ) {
                const size_t num_streams = json_streams.get<picojson::array>().size();
                for(size_t i=0; i<num_streams; ++i) {
                    const picojson::value& json_stream = json_streams.get(i);
                    if(json_stream.is<picojson::object>()) {
                        const picojson::value& json_encoding = json_stream.get("encoding");
                        const picojson::value& json_width    = json_stream.get("width");
                        const picojson::value& json_height   = json_stream.get("height");
                        const picojson::value& json_pitch    = json_stream.get("pitch");
                        const picojson::value& json_offset   = json_stream.get("offset");

                        if(json_encoding.is<std::string>() && json_width.is<int64_t>() &&
                                json_height.is<int64_t>() && json_pitch.is<int64_t>() &&
                                json_offset.is<int64_t>() )
                        {
                            StreamInfo si(
                                VideoFormatFromString(json_encoding.get<std::string>()),
                                json_width.get<int64_t>(),
                                json_height.get<int64_t>(),
                                json_pitch.get<int64_t>(),
                                (unsigned char*)0 + json_offset.get<int64_t>()
                            );

                            size_bytes += si.SizeBytes();
                            streams.push_back(si);
                        }
                    }
                }
            }
        }
    }
}

}
