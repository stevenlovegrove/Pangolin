#ifndef PANGOLIN_CUDAGL_H
#define PANGOLIN_CUDAGL_H

// Add the definition -DCUDA_BUILD to your project to include CUDA utilities
#ifdef CUDA_BUILD

#include <GL/glew.h>

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

enum GlCudaMappedBufferUse
{
  GlCudaMappedBufferAny = cudaGraphicsMapFlagsNone,
  GlCudaMappedBufferReadOnly = cudaGraphicsMapFlagsReadOnly,
  GlCudaMappedBufferWriteDiscard = cudaGraphicsMapFlagsWriteDiscard,
};

enum GlBufferType
{
  GlArrayBuffer = GL_ARRAY_BUFFER,
  GlElementArrayBuffer = GL_ARRAY_BUFFER,
  GlPixelPackBuffer = GL_PIXEL_PACK_BUFFER,
  GlPixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER
};

struct GlCudaRegisteredBuffer
{
  GlCudaRegisteredBuffer(GlBufferType buffer_type, GLsizeiptr size_bytes, GlCudaMappedBufferUse cudause = GlCudaMappedBufferAny, GLenum gluse = GL_DYNAMIC_DRAW );
  ~GlCudaRegisteredBuffer();
  void Bind() const;
  void UnBind() const;
  void Upload(const GLvoid* data, GLsizeiptr size_bytes, GLintptr offset = 0);
  GLuint bo;
  cudaGraphicsResource* bo_cuda;
  GlBufferType buffer_type;
};

struct CudaScopedMappedResource
{
  CudaScopedMappedResource(GlCudaRegisteredBuffer& buffer);
  ~CudaScopedMappedResource();
  void* operator*();
  GlCudaRegisteredBuffer& buffer;
};


////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

inline GlCudaRegisteredBuffer::GlCudaRegisteredBuffer(GlBufferType buffer_type, GLsizeiptr size_bytes, GlCudaMappedBufferUse cudause, GLenum gluse)
  : buffer_type(buffer_type)
{
  glGenBuffers(1, &bo);
  Bind();
  glBufferData(buffer_type, size_bytes, 0, gluse);
  UnBind();
  cudaGraphicsGLRegisterBuffer( &bo_cuda, bo, cudause );
}

inline GlCudaRegisteredBuffer::~GlCudaRegisteredBuffer()
{
  cudaGraphicsUnregisterResource(bo_cuda);
  glDeleteBuffers(1, &bo);
}

inline void GlCudaRegisteredBuffer::Bind() const
{
  glBindBuffer(buffer_type, bo);
}

inline void GlCudaRegisteredBuffer::UnBind() const
{
  glBindBuffer(buffer_type, 0);
}


inline void GlCudaRegisteredBuffer::Upload(const GLvoid* data, GLsizeiptr size_bytes, GLintptr offset)
{
  Bind();
  glBufferSubData(buffer_type,offset,size_bytes, data);
}

inline CudaScopedMappedResource::CudaScopedMappedResource(GlCudaRegisteredBuffer& buffer)
  : buffer(buffer)
{
  cudaGraphicsMapResources(1, &buffer.bo_cuda, 0);
}

inline CudaScopedMappedResource::~CudaScopedMappedResource()
{
  cudaGraphicsUnmapResources(1, &buffer.bo_cuda, 0);
}

inline void* CudaScopedMappedResource::operator*()
{
    size_t num_bytes;
    void* d_ptr;
    cudaGraphicsResourceGetMappedPointer(&d_ptr,&num_bytes,buffer.bo_cuda);
    return d_ptr;
}


}

#endif // CUDA_BUILD
#endif // PANGOLIN_CUDAGL_H
