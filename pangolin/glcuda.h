#ifndef PANGOLIN_CUDAGL_H
#define PANGOLIN_CUDAGL_H

// Add the definition -DCUDA_BUILD to your project to include CUDA utilities
#ifdef CUDA_BUILD

#include <GL/glew.h>

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

#include "gl.h"

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

enum GlBufferType
{
  GlArrayBuffer = GL_ARRAY_BUFFER,
  GlElementArrayBuffer = GL_ARRAY_BUFFER,
  GlPixelPackBuffer = GL_PIXEL_PACK_BUFFER,
  GlPixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER
};

struct GlBufferCudaPtr
{
  GlBufferCudaPtr(GlBufferType buffer_type, GLsizeiptr size_bytes, unsigned int cudause = cudaGraphicsMapFlagsNone, GLenum gluse = GL_DYNAMIC_DRAW );
  ~GlBufferCudaPtr();
  void Bind() const;
  void Unbind() const;
  void Upload(const GLvoid* data, GLsizeiptr size_bytes, GLintptr offset = 0);
  GLuint bo;
  cudaGraphicsResource* cuda_res;
  GlBufferType buffer_type;
};

struct GlTextureCudaArray : GlTexture
{
  GlTextureCudaArray(int width, int height, GLint internal_format);
  ~GlTextureCudaArray();
  cudaGraphicsResource* cuda_res;
};

struct CudaScopedMappedPtr
{
  CudaScopedMappedPtr(GlBufferCudaPtr& buffer);
  ~CudaScopedMappedPtr();
  void* operator*();
  cudaGraphicsResource* res;
};

struct CudaScopedMappedArray
{
  CudaScopedMappedArray(GlTextureCudaArray& tex);
  ~CudaScopedMappedArray();
  cudaArray* operator*();
  cudaGraphicsResource* res;
};

void CopyPboToTex(GlBufferCudaPtr& buffer, GlTexture& tex);

void swap(GlBufferCudaPtr& a, GlBufferCudaPtr& b);

////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

inline GlBufferCudaPtr::GlBufferCudaPtr(GlBufferType buffer_type, GLsizeiptr size_bytes, unsigned int cudause, GLenum gluse)
  : buffer_type(buffer_type)
{
  glGenBuffers(1, &bo);
  Bind();
  glBufferData(buffer_type, size_bytes, 0, gluse);
  Unbind();
  cudaGraphicsGLRegisterBuffer( &cuda_res, bo, cudause );
}

inline GlBufferCudaPtr::~GlBufferCudaPtr()
{
  cudaGraphicsUnregisterResource(cuda_res);
  glDeleteBuffers(1, &bo);
}

inline void GlBufferCudaPtr::Bind() const
{
  glBindBuffer(buffer_type, bo);
}

inline void GlBufferCudaPtr::Unbind() const
{
  glBindBuffer(buffer_type, 0);
}

inline void GlBufferCudaPtr::Upload(const GLvoid* data, GLsizeiptr size_bytes, GLintptr offset)
{
  Bind();
  glBufferSubData(buffer_type,offset,size_bytes, data);
}

inline GlTextureCudaArray::GlTextureCudaArray(int width, int height, GLint internal_format)
  :GlTexture(width,height,internal_format)
{
  // TODO: specify flags too
  cudaGraphicsGLRegisterImage(&cuda_res, tid, GL_TEXTURE_2D, cudaGraphicsMapFlagsNone);
}

inline GlTextureCudaArray::~GlTextureCudaArray()
{
  cudaGraphicsUnregisterResource(cuda_res);
}

inline CudaScopedMappedPtr::CudaScopedMappedPtr(GlBufferCudaPtr& buffer)
  : res(buffer.cuda_res)
{
  cudaGraphicsMapResources(1, &res, 0);
}

inline CudaScopedMappedPtr::~CudaScopedMappedPtr()
{
  cudaGraphicsUnmapResources(1, &res, 0);
}

inline void* CudaScopedMappedPtr::operator*()
{
    size_t num_bytes;
    void* d_ptr;
    cudaGraphicsResourceGetMappedPointer(&d_ptr,&num_bytes,res);
    return d_ptr;
}

inline CudaScopedMappedArray::CudaScopedMappedArray(GlTextureCudaArray& tex)
  : res(tex.cuda_res)
{
  cudaGraphicsMapResources(1, &res);
}

inline CudaScopedMappedArray::~CudaScopedMappedArray()
{
  cudaGraphicsUnmapResources(1, &res);
}

inline cudaArray* CudaScopedMappedArray::operator*()
{
  cudaArray* array;
  cudaGraphicsSubResourceGetMappedArray(&array, res, 0, 0);
  return array;
}

inline void CopyPboToTex(const GlBufferCudaPtr& buffer, GlTexture& tex, GLenum buffer_layout, GLenum buffer_data_type )
{
  buffer.Bind();
  tex.Bind();
  glTexImage2D(GL_TEXTURE_2D, 0, tex.internal_format, tex.width, tex.height, 0, buffer_layout, buffer_data_type, 0);
  buffer.Unbind();
}

inline void swap(GlBufferCudaPtr& a, GlBufferCudaPtr& b)
{
  std::swap(a.bo, b.bo);
  std::swap(a.cuda_res, b.cuda_res);
  std::swap(a.buffer_type, b.buffer_type);
}


}

#endif // CUDA_BUILD
#endif // PANGOLIN_CUDAGL_H
