#include <pangolin/gl/gl_type_info.h>
#include <pangolin/render/device_buffer.h>
#include <pangolin/utils/shared.h>

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
  struct QueueData {
    SharedDataPackage data;
    size_t dest_element;
  };

  DeviceGlBuffer(DeviceBuffer::Params params) :
      gluse_(GL_DYNAMIC_DRAW),
      grow_to_fit_(params.grow_to_fit),
      allow_retyping_(params.allow_retyping),
      num_elements_(0),
      num_elements_capacity_(params.min_element_size),
      gl_id_(0)
  {
    switch (params.kind) {
      case DeviceBuffer::Kind::VertexIndices:
        buffer_type_ = GL_ELEMENT_ARRAY_BUFFER;
        break;
      case DeviceBuffer::Kind::VertexAttributes:
        buffer_type_ = GL_ARRAY_BUFFER;
        break;
    };

    if (params.data) {
      queueUpdate(*params.data, 0);
      params.data = std::nullopt;
    }
  }

  ~DeviceGlBuffer() { free(); }

  void free() const
  {
    if (gl_id_ != 0) glDeleteBuffers(1, &gl_id_);
  }

  ScopedBind<DeviceBuffer> bind() const override
  {
    PANGO_CHECK(allocated(), "Attempting to bind uninitialized DeviceBuffer");

    return {
        [t = this->buffer_type_, id = this->gl_id_]() { glBindBuffer(t, id); },
        [t = this->buffer_type_]() { glBindBuffer(t, 0); },
    };
  }

  void checkRequest(const SharedDataPackage& data, size_t dest_element) const
  {
    const size_t needed_size = dest_element + data.num_elements;
    const bool needs_to_grow = capacity() < needed_size;
    const bool needs_retype = !dataType() || !(data.data_type == *dataType());

    PANGO_ENSURE(data.data);
    PANGO_ENSURE(grow_to_fit_ || !needs_to_grow);
    PANGO_ENSURE(allow_retyping_ || !allocated() || !needs_retype);
  }

  void queueUpdate(const SharedDataPackage& data, size_t dest_element) override
  {
    if (!data.num_elements) return;
    checkRequest(data, dest_element);
    std::lock_guard<std::recursive_mutex> guard(buffer_mutex_);
    updates_.push_back({data, dest_element});
  }

  bool empty() const override { return size() == 0; }

  bool allocated() const { return bool(dataType()); }

  std::optional<sophus::PixelFormat> dataType() const override
  {
    return data_type_;
  }

  size_t size() const override { return num_elements_; }

  size_t capacity() const override { return num_elements_capacity_; }

  void reserve(size_t elements) const override
  {
    num_elements_capacity_ = std::max(num_elements_capacity_, elements);
  }

  size_t sizeBytes() const override
  {
    return num_elements_capacity_ * elementSizeBytes();
  }

  void sync() const override
  {
    while (true) {
      QueueData u;
      {
        std::lock_guard<std::recursive_mutex> guard(buffer_mutex_);
        if (updates_.empty()) return;
        u = updates_.front();
        updates_.pop_front();
      }
      applyUpdateNow(u);
    }
  }

  void applyUpdateNow(const QueueData& u) const
  {
    // We already checked if these are compatible with policies on entry to
    // queue
    const size_t needed_size = u.dest_element + u.data.num_elements;
    const bool needs_to_grow = capacity() < needed_size;
    const bool needs_retype = !dataType() || !(u.data.data_type == *dataType());

    if (!allocated() || needs_retype || needs_to_grow) {
      GLuint backup_id = 0;
      size_t backup_size = 0;

      if (needs_to_grow && !needs_retype) {
        PANGO_CHECK(needs_to_grow);
        backup_id = gl_id_;
        backup_size = std::min(u.dest_element, size());
        gl_id_ = 0;
      } else {
        free();
      }

      data_type_ = u.data.data_type;
      num_elements_ = needed_size;
      num_elements_capacity_ =
          std::max(num_elements_capacity_, needed_size * 2);

      const size_t capacity_bytes = num_elements_capacity_ * elementSizeBytes();
      PANGO_ENSURE(capacity_bytes);
      PANGO_GL(glGenBuffers(1, &gl_id_));
      PANGO_GL(glBindBuffer(buffer_type_, gl_id_));
      PANGO_GL(glBufferData(buffer_type_, capacity_bytes, nullptr, gluse_));

      if (backup_id) {
        // restore previous data and delete old one
        PANGO_GL(glBindBuffer(GL_COPY_READ_BUFFER, backup_id));
        PANGO_GL(glBindBuffer(GL_COPY_WRITE_BUFFER, gl_id_));
        PANGO_GL(glCopyBufferSubData(
            GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
            backup_size * elementSizeBytes()));
        PANGO_GL(glDeleteBuffers(1, &backup_id));
        PANGO_GL(glBindBuffer(GL_COPY_READ_BUFFER, 0));
        PANGO_GL(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
        PANGO_GL(glBindBuffer(buffer_type_, gl_id_));
      }
    } else {
      PANGO_CHECK(needed_size <= num_elements_capacity_);
      PANGO_ENSURE(u.data.data_type == dataType());

      // Grow if within capacity if needed.
      num_elements_ = std::max(num_elements_, needed_size);
      PANGO_GL(glBindBuffer(buffer_type_, gl_id_));
    }

    PANGO_GL(glBufferSubData(
        buffer_type_, elementSizeBytes() * u.dest_element,
        elementSizeBytes() * u.data.num_elements, u.data.data.get()));

    PANGO_GL(glBindBuffer(buffer_type_, 0));
  }

  size_t elementSizeBytes() const
  {
    return data_type_ ? data_type_->numBytesPerPixel() : 0;
  }

  GLenum buffer_type_ = 0;
  GLenum gluse_ = 0;
  bool grow_to_fit_;
  bool allow_retyping_;
  mutable std::recursive_mutex buffer_mutex_;
  mutable std::deque<QueueData> updates_;
  mutable std::optional<PixelFormat> data_type_;
  mutable size_t num_elements_;
  mutable size_t num_elements_capacity_;
  mutable GLuint gl_id_;
};

Shared<DeviceBuffer> DeviceBuffer::Create(DeviceBuffer::Params p)
{
  return Shared<DeviceGlBuffer>::make(p);
}

template <>
ScopedBind<DeviceBuffer>::pScopedBind&
ScopedBind<DeviceBuffer>::getLocalActiveScopePtr()
{
  static thread_local pScopedBind x = nullptr;
  return x;
}

}  // namespace pangolin
