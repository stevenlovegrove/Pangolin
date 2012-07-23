/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#ifndef PANGOLIN_CUDAGL_H
#define PANGOLIN_CUDAGL_H

#include <algorithm>
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
  GlElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER,
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

private:
  GlBufferCudaPtr(const GlBufferCudaPtr&) {}
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

private:
  CudaScopedMappedPtr(const CudaScopedMappedPtr&) {}
};

struct CudaScopedMappedArray
{
  CudaScopedMappedArray(GlTextureCudaArray& tex);
  ~CudaScopedMappedArray();
  cudaArray* operator*();
  cudaGraphicsResource* res;

private:
  CudaScopedMappedArray(const CudaScopedMappedArray&) {}
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
  const cudaError_t err = cudaGraphicsGLRegisterImage(&cuda_res, tid, GL_TEXTURE_2D, cudaGraphicsMapFlagsNone);
  if( err != cudaSuccess ) {
      std::cout << "cudaGraphicsGLRegisterImage failed: " << err << std::endl;
  }
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
  tex.Unbind();
}

template<typename T>
inline void CopyDevMemtoTex(T* d_img, size_t pitch, GlTextureCudaArray& tex )
{
  CudaScopedMappedArray arr_tex(tex);
  cudaMemcpy2DToArray(*arr_tex, 0, 0, d_img, pitch, tex.width*sizeof(T), tex.height, cudaMemcpyDeviceToDevice );
}

inline void swap(GlBufferCudaPtr& a, GlBufferCudaPtr& b)
{
  std::swap(a.bo, b.bo);
  std::swap(a.cuda_res, b.cuda_res);
  std::swap(a.buffer_type, b.buffer_type);
}


}

#endif // PANGOLIN_CUDAGL_H
