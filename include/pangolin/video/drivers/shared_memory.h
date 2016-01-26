#pragma once

#include <pangolin/pangolin.h>
#include <pangolin/video/video.h>
#include <pangolin/utils/condition_variable.h>
#include <pangolin/utils/shared_memory_buffer.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace pangolin
{

class SharedMemoryVideo : public VideoInterface
{
public:
  SharedMemoryVideo(size_t w, size_t h, std::string pix_fmt,
    std::unique_ptr<SharedMemoryBufferInterface>&& shared_memory,
    std::unique_ptr<ConditionVariableInterface>&& buffer_full);
  ~SharedMemoryVideo();

  size_t SizeBytes() const;
  const std::vector<StreamInfo>& Streams() const;
  void Start();
  void Stop();
  bool GrabNext(unsigned char *image, bool wait);
  bool GrabNewest(unsigned char *image, bool wait);

private:
  VideoPixelFormat _fmt;
  size_t _w;
  size_t _h;
  size_t _frame_size;
  std::vector<StreamInfo> _streams;
  std::unique_ptr<SharedMemoryBufferInterface> _shared_memory;
  std::unique_ptr<ConditionVariableInterface> _buffer_full;
};

}
