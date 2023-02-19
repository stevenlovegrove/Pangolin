#pragma once

#include "shared_data_package.h"

#include <pangolin/render/extra_pixel_traits.h>
#include <pangolin/utils/scoped_bind.h>
#include <pangolin/utils/shared.h>
#include <sophus/image/dyn_image_types.h>

namespace pangolin
{

// Represents a std::vector-like device buffer, with type-erasure
struct DeviceBuffer {
  enum class Kind { VertexIndices, VertexAttributes };

  virtual ~DeviceBuffer(){};

  virtual ScopedBind<DeviceBuffer> bind() const = 0;

  // Returns true iff size() == 0
  virtual bool empty() const = 0;

  // Return the type for the stored data, if allocated, otherwise nullopt
  virtual std::optional<sophus::PixelFormat> dataType() const = 0;

  // size in number of stored dataType() elements
  virtual size_t size() const = 0;

  // Maximum size (in elements) before reallocation would occur
  virtual size_t capacity() const = 0;

  // Set capacity for next allocation
  virtual void reserve(size_t) const = 0;

  // size of allocated buffer in bytes. Will be at least `capacity() *
  // dataType().bytesPerPixel()`.
  virtual size_t sizeBytes() const = 0;

  // Queue upload of data into position `dest_element` of device buffer. The
  // data will be uploaded on the next call to `sync()`. In the future, may be
  // pipelined asynchronously, but completion is only guarenteed at the next
  // sync() call. Safe to call from any thread.
  //
  // When (dest_element + data.num_elements) extends beyond the current size(),
  // size() is increased until capacity(). If `Params::grow_to_fit` is true, a
  // reallocation and copy will occur, otherwise an error is thrown.
  //
  // If `allow_retyping` is true, data can differ from dataType() in type, but
  // existing data will be cleared and size will be set to dest_element +
  // data.num_elements, otherwise an error is thrown

  // preconditions:
  //  - data.data != nullptr;
  //  - if !grow_to_fit then data.num_elements + dest_element < capacity()
  //  - if !allow_retyping then `dataType()` != nullopt, then data.data_type
  //  must equal `dataType()`.
  virtual void queueUpdate(
      const SharedDataPackage& data, size_t dest_element = 0) = 0;

  // Upload pending data in device queue now (blocking).
  virtual void sync() const = 0;

  struct Params {
    Kind kind = Kind::VertexAttributes;
    size_t min_element_size = 0;
    bool grow_to_fit = true;
    bool allow_retyping = true;
    std::optional<SharedDataPackage> data = std::nullopt;
  };
  static Shared<DeviceBuffer> Create(Params p);
};

}  // namespace pangolin
