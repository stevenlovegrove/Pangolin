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

#include <pangolin/factory/factory_registry.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/video/drivers/images.h>
#include <pangolin/video/iostream_operators.h>

#include <cstring>
#include <fstream>

namespace pangolin
{

bool ImagesVideo::LoadFrame(size_t i)
{
    if( i < num_files) {
        Frame& frame = loaded[i];
        for(size_t c=0; c< num_channels; ++c) {
            const std::string& filename = Filename(i,c);
            const ImageFileType file_type = FileType(filename);

            if(file_type == ImageFileTypeUnknown && unknowns_are_raw) {
                // if raw_pitch is zero, assume image is packed.
                const size_t pitch = raw_pitch ? raw_pitch : raw_fmt.bpp * raw_width / 8;
                frame.push_back( LoadImage( filename, raw_fmt, raw_width, raw_height, pitch, raw_offset, raw_planes) );
            }else{
                frame.push_back( LoadImage( filename, file_type ) );
            }
        }
        return true;
    }
    return false;
}

void ImagesVideo::PopulateFilenamesFromJson(const std::string& filename)
{
    std::ifstream ifs( PathExpand(filename));
    picojson::value json;
    const std::string err = picojson::parse(json, ifs);
    if(err.empty()) {
        const std::string folder = PathParent(filename) + "/";
        device_properties = json["device_properties"];
        json_frames = json["frames"];

        num_files = json_frames.size();
        if(num_files == 0) {
            throw VideoException("Empty Json Image archive.");
        }

        num_channels = json_frames[0]["stream_files"].size();
        if(num_channels == 0) {
            throw VideoException("Empty Json Image archive.");
        }

        filenames.resize(num_channels);
        for(size_t c=0; c < num_channels; ++c) {
            filenames[c].resize(num_files);
            for(size_t i = 0; i < num_files; ++i) {
                const std::string path = json_frames[i]["stream_files"][c].get<std::string>();
                filenames[c][i] = (path.size() && path[0] == '/') ? path : (folder + path);
            }
        }
        loaded.resize(num_files);
    }else{
        throw VideoException(err);
    }
}

void ImagesVideo::PopulateFilenames(const std::string& wildcard_path)
{
    const std::vector<std::string> wildcards = Expand(wildcard_path, '[', ']', ',');
    num_channels = wildcards.size();

    if(wildcards.size() == 1 ) {
        const std::string expanded_path = PathExpand(wildcards[0]);
        const std::string possible_archive_path = expanded_path + "/archive.json";

        if (FileLowercaseExtention(expanded_path) == ".json" ) {
            PopulateFilenamesFromJson(wildcards[0]);
            return;
        }else if(FileExists(possible_archive_path)){
            PopulateFilenamesFromJson(possible_archive_path);
            return;
        }
    }

    filenames.resize(num_channels);

    for(size_t i = 0; i < wildcards.size(); ++i) {
        const std::string channel_wildcard = PathExpand(wildcards[i]);
        FilesMatchingWildcard(channel_wildcard, filenames[i],  SortMethod::NATURAL);
        if(num_files == size_t(-1)) {
            num_files = filenames[i].size();
        }else{
            if( num_files != filenames[i].size() ) {
                std::cerr << "Warning: Video Channels have unequal number of files" << std::endl;
            }
            num_files = std::min(num_files, filenames[i].size());
        }
        if(num_files == 0) {
            throw VideoException("No files found for wildcard '" + channel_wildcard + "'");
        }
    }

    // Resize empty frames vector to hold future images.
    loaded.resize(num_files);
}

void ImagesVideo::ConfigureStreamSizes()
{
    size_bytes = 0;
    for(size_t c=0; c < num_channels; ++c) {
        const TypedImage& img = loaded[0][c];
        const StreamInfo stream_info(img.fmt, img.w, img.h, img.pitch, (unsigned char*)(size_bytes));
        streams.push_back(stream_info);
        size_bytes += img.h*img.pitch;
    }
}

ImagesVideo::ImagesVideo(const std::string& wildcard_path)
    : num_files(-1), num_channels(0), next_frame_id(0),
      unknowns_are_raw(false)
{
    // Work out which files to sequence
    PopulateFilenames(wildcard_path);

    // Load first image in order to determine stream sizes etc
    LoadFrame(next_frame_id);

    ConfigureStreamSizes();

    // TODO: Queue frames in another thread.
}

ImagesVideo::ImagesVideo(
    const std::string& wildcard_path,
    const PixelFormat& raw_fmt,
    size_t raw_width, size_t raw_height,
    size_t raw_pitch, size_t raw_offset,
    size_t raw_planes
) : num_files(-1), num_channels(0), next_frame_id(0),
    unknowns_are_raw(true), raw_fmt(raw_fmt),
    raw_width(raw_width), raw_height(raw_height),
    raw_planes(raw_planes), raw_pitch(raw_pitch),
    raw_offset(raw_offset)
{
    // Work out which files to sequence
    PopulateFilenames(wildcard_path);

    // Load first image in order to determine stream sizes etc
    LoadFrame(next_frame_id);

    ConfigureStreamSizes();

    // TODO: Queue frames in another thread.
}

ImagesVideo::~ImagesVideo()
{
}

//! Implement VideoInput::Start()
void ImagesVideo::Start()
{

}

//! Implement VideoInput::Stop()
void ImagesVideo::Stop()
{

}

//! Implement VideoInput::SizeBytes()
size_t ImagesVideo::SizeBytes() const
{
    return size_bytes;
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& ImagesVideo::Streams() const
{
    return streams;
}

//! Implement VideoInput::GrabNext()
bool ImagesVideo::GrabNext( unsigned char* image, bool /*wait*/ )
{
    if(next_frame_id < loaded.size()) {
        Frame& frame = loaded[next_frame_id];

        if(frame.size() != num_channels) {
            LoadFrame(next_frame_id);
        }

        for(size_t c=0; c < num_channels; ++c){
            TypedImage& img = frame[c];
            if(!img.ptr || img.w != streams[c].Width() || img.h != streams[c].Height() ) {
                return false;
            }
            const StreamInfo& si = streams[c];
            std::memcpy(image + (size_t)si.Offset(), img.ptr, si.SizeBytes());
            img.Deallocate();
        }
        frame.clear();

        next_frame_id++;
        return true;
    }

    return false;
}

//! Implement VideoInput::GrabNewest()
bool ImagesVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image,wait);
}

size_t ImagesVideo::GetCurrentFrameId() const
{
    return (int)next_frame_id - 1;
}

size_t ImagesVideo::GetTotalFrames() const
{
    return num_files;
}

size_t ImagesVideo::Seek(size_t frameid)
{
    next_frame_id = std::max(size_t(0), std::min(frameid, num_files));
    return next_frame_id;
}

const picojson::value& ImagesVideo::DeviceProperties() const
{
    return device_properties;
}

const picojson::value& ImagesVideo::FrameProperties() const
{
    const size_t frame = GetCurrentFrameId();

    if( json_frames.evaluate_as_boolean() && frame < json_frames.size()) {
        const picojson::value& frame_props = json_frames[frame];
        if(frame_props.contains("frame_properties")) {
            return frame_props["frame_properties"];
        }
    }

    return null_props;
}

PANGOLIN_REGISTER_FACTORY(ImagesVideo)
{
    struct ImagesVideoVideoFactory final : public TypedFactoryInterface<VideoInterface> {
        std::map<std::string,Precedence> Schemes() const override
        {
            return {{"file",20}, {"files",20}, {"image",10}, {"images",10}};
        }
        const char* Description() const override
        {
            return "Load an image collection as a video. Supports one or more synchronized streams. Use images://[wildcard1,wildcard2,...] to specify multiple channels. Wildcard can contain ? or * to match one or many charectors.";
        }
        ParamSet Params() const override
        {
            return {{
                {"fmt","GRAY8","RAW files only. Pixel format, see pixel format help for all possible values"},
                {"size","640x480","RAW files only. Image size, required if fmt is specified"},
                {"pitch","0","RAW files only. Specify distance from the start of one row to the next in bytes. If not specified, assumed image is packed."},
                {"offset","0","Offset from the start of the file in bytes where the image starts"},
                {"planes","1","Number of channel planes (outer array channels) for raw image. fmt should be the format of an element in the individual plane."}
            }};
        }
        std::unique_ptr<VideoInterface> Open(const Uri& uri) override {
            ParamReader reader(Params(),uri);

            const bool raw = reader.Contains("fmt");
            const std::string path = PathExpand(uri.url);

            if(raw) {
                const std::string sfmt = reader.Get<std::string>("fmt");
                const PixelFormat fmt = PixelFormatFromString(sfmt);
                const ImageDim dim = reader.Get<ImageDim>("size");
                const size_t image_pitch = reader.Get<int>("pitch");
                const size_t image_offset = reader.Get<int>("offset");
                const size_t image_planes = reader.Get<int>("planes");
                return std::unique_ptr<VideoInterface>( new ImagesVideo(
                    path, fmt, dim.x, dim.y, image_pitch, image_offset, image_planes
                ));
            }else{
                return std::unique_ptr<VideoInterface>( new ImagesVideo(path) );
            }
        }
    };

    return FactoryRegistry::I()->RegisterFactory<VideoInterface>(std::make_shared<ImagesVideoVideoFactory>());
}

}
