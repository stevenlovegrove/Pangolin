#include <pangolin/image/runtime_image.h>
#include <pangolin/platform.h>

#ifdef BUILD_PANGOLIN_VIDEO
#include <pangolin/video/drivers/pango.h>
#include <pangolin/video/drivers/pango_video_output.h>
#endif

namespace pangolin
{

IntensityImage<> LoadPango(std::string const& uri)
{
  PANGOLIN_UNUSED(uri);

#ifdef BUILD_PANGOLIN_VIDEO
  std::unique_ptr<VideoInterface> video = OpenVideo(uri);
  if (!video || video->Streams().size() != 1) {
    throw pangolin::VideoException(
        "Wrong number of streams: exactly one expected.");
  }

  std::unique_ptr<uint8_t[]> buffer(new uint8_t[video->SizeBytes()]);
  StreamInfo const& stream_info = video->Streams()[0];

  // Grab first image from video
  if (!video->GrabNext(buffer.get(), true)) {
    throw pangolin::VideoException("Failed to grab image from stream");
  }

  // Allocate storage for user image to return
  TypedImage image(
      stream_info.Width(), stream_info.Height(), stream_info.PixFormat());

  // Copy image data into user buffer.
  Image<unsigned char> const img = stream_info.StreamImage(buffer.get());
  PANGO_ENSURE(image.pitch <= img.pitch);
  for (size_t y = 0; y < image.h; ++y) {
    std::memcpy(image.RowPtr(y), img.RowPtr(y), image.pitch);
  }

  return image;
#else
  throw std::runtime_error(
      "Video Support not enabled. Please rebuild Pangolin.");
#endif
}

void SavePango(
    IntensityImage<> const& image, std::string const& uri,
    bool /*top_line_first*/)
{
  PANGOLIN_UNUSED(image);
  PANGOLIN_UNUSED(uri);

#ifdef BUILD_PANGOLIN_VIDEO
  std::unique_ptr<VideoOutputInterface> video = OpenVideoOutput(uri);
  StreamInfo stream(fmt, image.w, image.h, image.pitch);
  video->SetStreams({stream});
  video->WriteStreams(image.ptr);
#else
  throw std::runtime_error(
      "Video Support not enabled. Please rebuild Pangolin.");
#endif
}

}  // namespace pangolin
