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

#include <pangolin/factory/factory_registry.h>
#include <pangolin/log/playback_session.h>
#include <pangolin/utils/file_extension.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/video/drivers/pango.h>
#include <pangolin/video/iostream_operators.h>

#include <functional>

namespace pangolin
{

const std::string pango_video_type = "raw_video";

PangoVideo::PangoVideo(const std::string& filename, bool realtime)
    : _filename(filename), _realtime(realtime)
{
    // Open shared reference to PacketStreamReader
    _reader = PlaybackSession::Default().Open(filename);

    PANGO_ENSURE(_reader);

    _src_id = FindPacketStreamSource();

    if(_src_id != -1) {
        SetupStreams(_reader->Sources()[_src_id]);
    }else{
        throw pangolin::VideoException("No appropriate video streams found in log.");
    }
}

PangoVideo::~PangoVideo()
{
}

size_t PangoVideo::SizeBytes() const
{
    return _size_bytes;
}

const std::vector<StreamInfo>& PangoVideo::Streams() const
{
    return _streams;
}

void PangoVideo::Start()
{

}

void PangoVideo::Stop()
{

}

bool PangoVideo::GrabNext(unsigned char* image, bool /*wait*/)
{
    _frame_properties = picojson::value();
    std::lock_guard<decltype(_reader->Mutex())> lg(_reader->Mutex());

    try
    {
        Packet fi = _reader->NextFrame(_src_id);
        _frame_properties = fi.meta;
        fi.ReadRaw(reinterpret_cast<char*>(image), _size_bytes);
        return true;
    }
    catch (std::exception& ex)
    {
        pango_print_warn("%s", ex.what());
        return false;
    }
    catch(...)
    {
        return false;
    }
}

bool PangoVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image, wait);
}

int PangoVideo::GetCurrentFrameId() const
{
    return static_cast<int>(_reader->GetPacketIndex(_src_id));
}

int PangoVideo::GetTotalFrames() const
{
    return static_cast<int>(_reader->GetNumPackets(_src_id));
}

int PangoVideo::Seek(int frameid)
{
    std::lock_guard<decltype(_reader->Mutex())> lg(_reader->Mutex());
    _frame_properties = picojson::value();
    return _reader->Seek(_src_id, frameid);
}

int PangoVideo::FindPacketStreamSource()
{
    for(const auto& src : _reader->Sources())
    {
        if (!src.driver.compare(pango_video_type))
        {
            return static_cast<int>(src.id);
        }
    }

    return -1;
}

void PangoVideo::SetupStreams(const PacketStreamSource& src)
{
    try {
        // Read sources header
        _size_bytes = src.data_size_bytes;

        _device_properties = src.info["device"];
        const picojson::value& json_streams = src.info["streams"];
        const size_t num_streams = json_streams.size();
        for (size_t i = 0; i < num_streams; ++i)
        {
            const picojson::value& json_stream = json_streams[i];
            StreamInfo si(
                    PixelFormatFromString(
                            json_stream["encoding"].get<std::string>()
                            ),
                    json_stream["width"].get<int64_t>(),
                    json_stream["height"].get<int64_t>(),
                    json_stream["pitch"].get<int64_t>(),
                    (unsigned char*) 0 + json_stream["offset"].get<int64_t>()
                            );

            _streams.push_back(si);
        }
    } catch (...)
    {
        pango_print_info("Unable to parse PacketStream Source. File version incompatible.\n");
    }
}

PANGOLIN_REGISTER_FACTORY(PangoVideo)
{
    struct PangoVideoFactory : public FactoryInterface<VideoInterface> {
        std::unique_ptr<VideoInterface> Open(const Uri& uri) override {
            const std::string path = PathExpand(uri.url);

            if( !uri.scheme.compare("pango") || FileType(uri.url) == ImageFileTypePango ) {
                const bool realtime = uri.Contains("realtime");
                return std::unique_ptr<VideoInterface>(new PangoVideo(path.c_str(), realtime));
            }
            return std::unique_ptr<VideoInterface>();
        }
    };

    auto factory = std::make_shared<PangoVideoFactory>();
    FactoryRegistry<VideoInterface>::I().RegisterFactory(factory, 10, "pango");
    FactoryRegistry<VideoInterface>::I().RegisterFactory(factory,  5, "file");
}

}
