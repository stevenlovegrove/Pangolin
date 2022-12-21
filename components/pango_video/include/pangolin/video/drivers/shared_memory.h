#pragma once

#include <pangolin/utils/posix/condition_variable.h>
#include <pangolin/utils/posix/shared_memory_buffer.h>
#include <pangolin/video/video_interface.h>

#include <memory>
#include <vector>

namespace pangolin
{

class SharedMemoryVideo : public VideoInterface
{
  public:
  SharedMemoryVideo(
      size_t w, size_t h, std::string pix_fmt,
      std::shared_ptr<SharedMemoryBufferInterface> const& shared_memory,
      std::shared_ptr<ConditionVariableInterface> const& buffer_full);
  ~SharedMemoryVideo();

  size_t SizeBytes() const;
  std::vector<StreamInfo> const& Streams() const;
  void Start();
  void Stop();
  bool GrabNext(unsigned char* image, bool wait);
  bool GrabNewest(unsigned char* image, bool wait);

  private:
  PixelFormat _fmt;
  size_t _frame_size;
  std::vector<StreamInfo> _streams;
  std::shared_ptr<SharedMemoryBufferInterface> _shared_memory;
  std::shared_ptr<ConditionVariableInterface> _buffer_full;
};

}  // namespace pangolin
