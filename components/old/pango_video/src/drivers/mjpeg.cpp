#include <pangolin/platform.h>
#include <pangolin/video/drivers/mjpeg.h>
#include <pangolin/video/video_exception.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/image/image_io.h>
#include <pangolin/utils/file_utils.h>

namespace pangolin
{

// this is defined in image_io_jpg.cpp but not in any public headers.
std::vector<std::streampos> GetMJpegOffsets(std::ifstream& is);

MjpegVideo::MjpegVideo(const std::string& filename)
{
    const std::string full_path = PathExpand(filename);
    if(!FileExists(full_path)) {
        throw VideoException("No such file, " + full_path);
    }

    const ImageFileType file_type = FileType(full_path);
    if(file_type != ImageFileType::ImageFileTypeJpg) {
        throw VideoException(full_path + " has no jpeg header when attempting to open as mjpeg stream.");
    }

    bFile.open( full_path.c_str(), std::ios::in | std::ios::binary );
    if(!bFile.is_open()) {
        throw VideoException("Unable to open " + full_path);
    }

    offsets = GetMJpegOffsets(bFile);

    next_image = LoadImage(bFile, ImageFileType::ImageFileTypeJpg);
    if(!next_image.IsValid()) {
        throw VideoException("Unable to load first jpeg in mjpeg stream");
    }

    streams.emplace_back(next_image.fmt, next_image.w, next_image.h, next_image.pitch, nullptr);
    size_bytes = next_image.SizeBytes();
}

MjpegVideo::~MjpegVideo()
{

}

//! Implement VideoInput::Start()
void MjpegVideo::Start()
{

}

//! Implement VideoInput::Stop()
void MjpegVideo::Stop()
{

}

//! Implement VideoInput::SizeBytes()
size_t MjpegVideo::SizeBytes() const
{
    return size_bytes;
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& MjpegVideo::Streams() const
{
    return streams;
}

bool MjpegVideo::LoadNext()
{
    if(!next_image.IsValid() && bFile.good()) {
        try {
            next_image = LoadImage(bFile, ImageFileType::ImageFileTypeJpg);
        }  catch (const std::runtime_error&) {
            return false;
        }
    }
    return next_image.IsValid();
}

//! Implement VideoInput::GrabNext()
bool MjpegVideo::GrabNext( unsigned char* image, bool wait )
{
    if( LoadNext() ) {
        memcpy(image, next_image.ptr, size_bytes);
        next_image.Deallocate();
        ++next_frame_id;
        return true;
    }
    return false;
}

//! Implement VideoInput::GrabNewest()
bool MjpegVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image, wait);
}

size_t MjpegVideo::GetCurrentFrameId() const
{
    return next_frame_id - 1;
}

size_t MjpegVideo::GetTotalFrames() const
{
    return offsets.size();
}

size_t MjpegVideo::Seek(size_t frameid)
{
    if(frameid != next_frame_id) {
        // Clamp to within range
        next_frame_id = std::min(frameid, offsets.size()-1);
        // Clear any eof markers etc
        bFile.clear();
        bFile.seekg(offsets[next_frame_id]);
        // Remove any cached image data
        next_image.Deallocate();
    }else{
        // Do nothing
    }
    return next_frame_id;
}


PANGOLIN_REGISTER_FACTORY(MjpegVideo)
{
    struct MjpegVideoFactory final : public TypedFactoryInterface<VideoInterface> {
        std::map<std::string,Precedence> Schemes() const override
        {
            return {{"mjpeg",0}};
        }
        const char* Description() const override
        {
            return "Load Motion Jpeg video streams";
        }
        ParamSet Params() const override
        {
            return {{
            }};
        }
        std::unique_ptr<VideoInterface> Open(const Uri& uri) override {
            return std::unique_ptr<VideoInterface>(new MjpegVideo(uri.url));
        }
    };

    return FactoryRegistry::I()->RegisterFactory<VideoInterface>(std::make_shared<MjpegVideoFactory>());
}

}
