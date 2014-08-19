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

#include <pangolin/video/drivers/pango_video_output.h>
#include <pangolin/utils/picojson.h>

namespace pangolin
{

const std::string pango_video_type = "pango_raw_video";

PangoVideoOutput::PangoVideoOutput(const std::string& filename)
    : packetstream(filename), packetstreamsrcid(-1)
{
}

PangoVideoOutput::~PangoVideoOutput()
{
}

const std::vector<StreamInfo>& PangoVideoOutput::Streams() const
{
    return streams;
}

void PangoVideoOutput::AddStreams(const std::vector<StreamInfo>& st)
{
    if(packetstreamsrcid == -1) {
        streams.insert(streams.begin(), st.begin(), st.end());
    }else{
        throw std::runtime_error("Unable to add new streams");
    }
}

void PangoVideoOutput::WriteHeader()
{
    picojson::value json_header(picojson::object_type,false);
    picojson::value json_streams(picojson::array_type,false);

    total_frame_size = 0;
    for(unsigned int i=0; i< streams.size(); ++i) {
        StreamInfo& si = streams[i];
        total_frame_size += si.SizeBytes();

        picojson::value json_stream(picojson::object_type,false);
        json_stream["encoding"] = picojson::value(si.PixFormat().format);
        json_stream["width"] = picojson::value( (int64_t)si.Width() );
        json_stream["height"] = picojson::value( (int64_t)si.Height() );
        json_stream["pitch"] = picojson::value( (int64_t)si.Pitch() );
        json_stream["offset"] = picojson::value( (int64_t)si.Offset() );
        json_streams.get<picojson::array>().push_back(json_stream);
    }

    json_header["streams"] = json_streams;

    picojson::value json_frame_streams(picojson::array_type,false);
    json_frame_streams.get<picojson::array>().push_back( picojson::value("uint8") );
    json_frame_streams.get<picojson::array>().push_back( picojson::value( (int64_t)total_frame_size) );

    picojson::value json_frame(picojson::object_type,false);
    json_frame["stream_data"] = json_frame_streams;

    packetstreamsrcid = packetstream.AddSource(
        pango_video_type,
        "default_uri",
        json_frame.serialize(),
        json_header.serialize()
    );
}

int PangoVideoOutput::WriteStreams(unsigned char* data)
{
    if(packetstreamsrcid == -1) {
        WriteHeader();
    }

    packetstream.WriteSourceFrame(
        packetstreamsrcid,
        (char*)data, total_frame_size
    );

    return 0;
}

}
