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

#include <pangolin/video/drivers/pango.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/video/iostream_operators.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/utils/file_extension.h>

#include <functional>

#ifndef _WIN_
#  include <unistd.h>
#endif

namespace pangolin
{

const std::string pango_video_type = "raw_video";

PangoVideo::PangoVideo(const std::string& filename, bool realtime)
    : _reader(filename), _filename(filename), _realtime(realtime),
      _is_pipe(pangolin::IsPipe(filename)),
      _is_pipe_open(true),
      _pipe_fd(-1)
{
    // N.B. is_pipe_open can default to true since the reader opens the file and
    // reads header information from it, which means the pipe must be open and
    // filled with data.
    _src_id = FindSource();

    if(_src_id == -1)
        throw pangolin::VideoException("No appropriate video streams found in log.");
}

PangoVideo::~PangoVideo()
{
#ifndef _WIN_
    if (_pipe_fd != -1)
        close(_pipe_fd);
#endif
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
    _frame_properties = json::value();
    std::lock_guard<decltype(_reader.Mutex())> lg(_reader.Mutex());

#ifndef _WIN_
    if (_is_pipe && !_is_pipe_open)
    {
        if (_pipe_fd == -1)
            _pipe_fd = ReadablePipeFileDescriptor(_filename);

        if (_pipe_fd == -1)
            return false;

        // Test whether the pipe has data to be read. If so, open the
        // file stream and start reading. After this point, the file
        // descriptor is owned by the reader.
        if (PipeHasDataToRead(_pipe_fd))
        {
            _reader.Open(_filename);
            close(_pipe_fd);
            _is_pipe_open = true;
        }
        else
            return false;
    }
#endif

    try
    {
        auto fi = _reader.NextFrame(_src_id, _realtime ? &_realtime_sync : nullptr);
        if (!fi.None())
        {
            //update metadata. This should not be stateful, but a higher level interface requires this.
            //todo eventually propagate the goodness up.
            _frame_properties = fi.meta;

            // read this frame's actual data
            _reader.ReadRaw(reinterpret_cast<char*>(image), _size_bytes);
            return true;
        }
        else
        {
            if (_is_pipe && !_reader.Good())
                HandlePipeClosed();
            return false;
        }
    }
    catch (std::exception& ex)
    {
        if (_is_pipe)
            HandlePipeClosed();

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
    return static_cast<int>(_reader.GetPacketIndex(_src_id));
}

int PangoVideo::GetTotalFrames() const
{
    return static_cast<int>(_reader.GetNumPackets(_src_id));
}

int PangoVideo::Seek(int frameid)
{
    std::lock_guard<decltype(_reader.Mutex())> lg(_reader.Mutex());
    _frame_properties = json::value(); //clear frame props

    auto fi = _reader.Seek(_src_id, frameid, _realtime ? &_realtime_sync : nullptr);

    if (fi.None())
        return -1;
    else
        return fi.sequence_num;
}

int PangoVideo::FindSource()
{
    for(const auto& src : _reader.Sources())
    {
        try
        {
            if (!src.driver.compare(pango_video_type))
            {
                // Read sources header
                _size_bytes = src.data_size_bytes;

                _device_properties = src.info["device"];
                const json::value& json_streams = src.info["streams"];
                const size_t num_streams = json_streams.size();
                for (size_t i = 0; i < num_streams; ++i)
                {
                    const json::value& json_stream = json_streams[i];
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

                return static_cast<int>(src.id);
            }
        }
        catch (...)
        {
            pango_print_info("Unable to parse PacketStream Source. File version incompatible.\n");
        }
    }

    return -1;
}

void PangoVideo::HandlePipeClosed()
{
    // The pipe was closed by the other end. The pipe will have to be
    // re-opened, but it is not desirable to block at this point.
    //
    // The next time a frame is grabbed, the pipe will be checked and if
    // it is open, the stream will be re-opened.
    _reader.Close();
    _is_pipe_open = false;
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
