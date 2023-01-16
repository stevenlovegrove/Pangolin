#include <pangolin/context/factory.h>
#include <pangolin/gl/gl_type_info.h>
#include <pangolin/render/device_buffer.h>

#include <deque>
#include <mutex>
#include <stdexcept>

using namespace sophus;

namespace pangolin
{

template <size_t kN, size_t kFrom>
std::array<size_t, kN> headAndZeros(const std::array<size_t, kFrom>& x)
{
  std::array<size_t, kN> ret;
  for (size_t i = 0; i < kN; ++i) {
    ret[i] = x[i];
  }
  for (size_t i = kN; i < kFrom; ++i) {
    if (x[i] != 0) {
      throw std::runtime_error("Last Dim+1 .. N elements must be zero");
    }
  }
  return ret;
}

struct DeviceGlBuffer : public DeviceBuffer {
  DeviceGlBuffer(GLenum buffer_type, GLenum gluse) :
      buffer_type_(buffer_type),
      gluse_(gluse),
      num_elements_(0),
      num_elements_capacity_(0),
      gl_id_(0)
  {
  }

  ~DeviceGlBuffer() { free(); }

  void free()
  {
    if (gl_id_ != 0) glDeleteBuffers(1, &gl_id_);
  }

  ScopedBind<DeviceBuffer> bind() const override
  {
    PANGO_CHECK(!empty(), "Attempting to bind uninitialized DeviceBuffer");

    return {
        [t = this->buffer_type_, id = this->gl_id_]() { glBindBuffer(t, id); },
        [t = this->buffer_type_]() { glBindBuffer(t, 0); },
    };
  }

  void pushToUpdateQueue(const Data& u) override
  {
    SOPHUS_ASSERT(u.data);
    // always true
    //SOPHUS_ASSERT_GE(u.num_elements, 0);

    std::lock_guard<std::recursive_mutex> guard(buffer_mutex_);
    updates_.push_back(u);
    // sync();
  }

  bool empty() const override { return gl_id_ == 0; }

  sophus::RuntimePixelType dataType() const override { return data_type_; }

  size_t numElements() const override { return num_elements_; }

  void sync() const override
  {
    while (true) {
      Data u;
      {
        std::lock_guard<std::recursive_mutex> guard(buffer_mutex_);
        if (updates_.empty()) return;
        u = updates_.front();
        updates_.pop_front();
      }
      applyUpdateNow(u);
    }
  }

  void applyUpdateNow(const Data& u) const
  {
    SOPHUS_ASSERT(u.data);
    // always true
    // SOPHUS_ASSERT_GE(u.num_elements, 0);
    if (u.num_elements == 0) {
      return;
    }

    if (gl_id_ == 0 /* || incompatible...  */) {
      data_type_ = u.data_type;
      num_elements_ = u.params.dest_element + u.num_elements;
      num_elements_capacity_ =
          std::max(num_elements_, u.params.num_reserve_elements);

      const size_t capacity_bytes = num_elements_capacity_ * elementSizeBytes();
      glGenBuffers(1, &gl_id_);
      glBindBuffer(buffer_type_, gl_id_);
      glBufferData(buffer_type_, capacity_bytes, nullptr, gluse_);
    } else {
      const size_t needed_size = u.params.dest_element + u.num_elements;
      PANGO_CHECK(
          u.data_type == data_type_,
          "Attempting to upload {} format for {} texture.", u.data_type,
          data_type_);
      PANGO_CHECK(needed_size <= num_elements_capacity_);
      // Grow if within capacity if needed.
      num_elements_ = std::max(num_elements_, needed_size);
      glBindBuffer(buffer_type_, gl_id_);
    }

    glBufferSubData(
        buffer_type_, elementSizeBytes() * u.params.dest_element,
        elementSizeBytes() * u.num_elements, u.data.get());

    glBindBuffer(buffer_type_, 0);
  }

  size_t elementSizeBytes() const
  {
    return data_type_.num_channels * data_type_.num_bytes_per_pixel_channel;
  }

  size_t sizeBytes() const { return num_elements_ * elementSizeBytes(); }

  GLenum buffer_type_ = 0;
  GLenum gluse_ = 0;
  mutable std::recursive_mutex buffer_mutex_;
  mutable std::deque<Data> updates_;
  mutable RuntimePixelType data_type_;
  mutable size_t num_elements_;
  mutable size_t num_elements_capacity_;
  mutable GLuint gl_id_;
};

PANGO_CREATE(DeviceBuffer)
{
  const GLenum bo_use = GL_DYNAMIC_DRAW;

  switch (p.kind) {
    case DeviceBuffer::Kind::VertexIndices:
      return Shared<DeviceGlBuffer>::make(GL_ELEMENT_ARRAY_BUFFER, bo_use);
    case DeviceBuffer::Kind::VertexAttributes:
      return Shared<DeviceGlBuffer>::make(GL_ARRAY_BUFFER, bo_use);
  };
  PANGO_UNREACHABLE();
}

template <>
ScopedBind<DeviceBuffer>::pScopedBind&
ScopedBind<DeviceBuffer>::getLocalActiveScopePtr()
{
  static thread_local pScopedBind x = nullptr;
  return x;
}

}  // namespace pangolin
