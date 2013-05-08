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

struct GlBufferCudaPtr : public GlBuffer
{
    GlBufferCudaPtr(GlBufferType buffer_type, GLuint num_elements, GLenum datatype, GLuint count_per_element, unsigned int cudause /*= cudaGraphicsMapFlagsNone*/, GLenum gluse /*= GL_DYNAMIC_DRAW*/ );
    GlBufferCudaPtr(GlBufferType buffer_type, GLuint size_bytes, unsigned int cudause /*= cudaGraphicsMapFlagsNone*/, GLenum gluse /*= GL_DYNAMIC_DRAW*/ );
    
    PANGOLIN_DEPRECATED
    GlBufferCudaPtr(GlBufferType buffer_type, GLuint width, GLuint height, GLenum datatype, GLuint count_per_element, unsigned int cudause /*= cudaGraphicsMapFlagsNone*/, GLenum gluse /*= GL_DYNAMIC_DRAW*/ );
    
    ~GlBufferCudaPtr();
    
    cudaGraphicsResource* cuda_res;
};

struct GlTextureCudaArray : GlTexture
{
    // Some internal_formats aren't accepted. I have trouble with GL_RGB8
    GlTextureCudaArray(int width, int height, GLint internal_format, bool sampling_linear = true);
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

inline GlBufferCudaPtr::GlBufferCudaPtr(GlBufferType buffer_type, GLuint num_elements, GLenum datatype, GLuint count_per_element, unsigned int cudause, GLenum gluse )
    : GlBuffer(buffer_type, num_elements, datatype, count_per_element, gluse)
{
    cudaGraphicsGLRegisterBuffer( &cuda_res, bo, cudause );
}

inline GlBufferCudaPtr::GlBufferCudaPtr(GlBufferType buffer_type, GLuint size_bytes, unsigned int cudause /*= cudaGraphicsMapFlagsNone*/, GLenum gluse /*= GL_DYNAMIC_DRAW*/ )
    : GlBuffer(buffer_type, size_bytes, GL_BYTE, 1, gluse)
{
    cudaGraphicsGLRegisterBuffer( &cuda_res, bo, cudause );
}

inline GlBufferCudaPtr::GlBufferCudaPtr(GlBufferType buffer_type, GLuint width, GLuint height, GLenum datatype, GLuint count_per_element, unsigned int cudause /*= cudaGraphicsMapFlagsNone*/, GLenum gluse /*= GL_DYNAMIC_DRAW*/ )
    : GlBuffer(buffer_type, width*height, datatype, count_per_element, gluse)
{
    cudaGraphicsGLRegisterBuffer( &cuda_res, bo, cudause );    
}

inline GlBufferCudaPtr::~GlBufferCudaPtr()
{
    cudaGraphicsUnregisterResource(cuda_res);
}

inline GlTextureCudaArray::GlTextureCudaArray(int width, int height, GLint internal_format, bool sampling_linear)
    :GlTexture(width,height,internal_format, sampling_linear)
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
