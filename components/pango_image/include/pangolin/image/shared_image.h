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

#pragma once

#include <pangolin/image/image.h>
#include <pangolin/image/copy.h>

namespace pangolin {

template<class T> using DefaultImageAllocator = std::allocator<T>;

// Image that manages it's own memory, storing a strong pointer to it's memory
template<typename T>
class SharedImage : public Image<T>
{
private:
    std::shared_ptr<T[]> shared_memory;

public:
    // Destructor
    inline
    ~SharedImage()
    {
    }

    // Null image
    inline
    SharedImage()
    {
    }

    // Row image
    inline
    SharedImage(size_t w)
        : Image<T>( nullptr, w, 1, w*sizeof(T) ),
          shared_memory(new T[w])
    {
        Image<T>::ptr = shared_memory.get();
    }

    inline
    SharedImage(size_t w, size_t h)
        : Image<T>( nullptr, w, h, w*sizeof(T) ),
          shared_memory(new T[w*h])
    {
        Image<T>::ptr = shared_memory.get();
    }

    inline
    SharedImage(size_t w, size_t h, size_t pitch_bytes)
        : Image<T>( nullptr, w, h, pitch_bytes),
          shared_memory(new T[h*pitch_bytes / sizeof(T)])
    {
        Image<T>::ptr = shared_memory.get();
    }

    // Copyable with shared semantics (not a deep copy).
    inline
    SharedImage( const SharedImage<T>& other ) = default;

    // Move constructor
    inline
    SharedImage(SharedImage<T>&& img)
    {
        *this = std::move(img);
    }

    // Move asignment
    inline
    void operator=(SharedImage<T>&& img)
    {
        Image<T>::pitch = img.pitch;
        Image<T>::ptr   = img.ptr;
        Image<T>::w     = img.w;
        Image<T>::h     = img.h;
        shared_memory = std::move(img.shared_memory);
        img.ptr = nullptr;
    }

    // Explicit copy constructor
    template<typename TOther>
    SharedImage( const CopyObject<TOther>& other )
    {
        CopyFrom(other.obj);
    }

    // Explicit copy assignment
    template<typename TOther>
    void operator=(const CopyObject<TOther>& other)
    {
        CopyFrom(other.obj);
    }

    inline
    void Swap(SharedImage<T>& img)
    {
        std::swap(img.pitch, Image<T>::pitch);
        std::swap(img.ptr, Image<T>::ptr);
        std::swap(img.w, Image<T>::w);
        std::swap(img.h, Image<T>::h);
        std::swap(img.shared_memory, shared_memory);
    }

    inline
    void CopyFrom(const Image<T>& img)
    {
        if(!Image<T>::IsValid() || Image<T>::w != img.w || Image<T>::h != img.h) {
            throw std::runtime_error("Incompatible image specification for copy");
        }
        Image<T>::CopyFrom(img);
    }

    inline void Deallocate()
    {
        shared_memory = nullptr;
        Image<T>::ptr = nullptr;
    }
};

}
