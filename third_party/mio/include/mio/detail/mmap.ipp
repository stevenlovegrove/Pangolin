/* Copyright 2017 https://github.com/mandreyel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MIO_BASIC_MMAP_IMPL
#define MIO_BASIC_MMAP_IMPL

#include "mio/mmap.hpp"
#include "mio/page.hpp"
#include "mio/detail/string_util.hpp"

#include <algorithm>

#ifndef _WIN32
# include <unistd.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
#endif

namespace mio {
namespace detail {

#ifdef _WIN32
namespace win {

/** Returns the 4 upper bytes of an 8-byte integer. */
inline DWORD int64_high(int64_t n) noexcept
{
    return n >> 32;
}

/** Returns the 4 lower bytes of an 8-byte integer. */
inline DWORD int64_low(int64_t n) noexcept
{
    return n & 0xffffffff;
}

template<
    typename String,
    typename = typename std::enable_if<
        std::is_same<typename char_type<String>::type, char>::value
    >::type
> file_handle_type open_file_helper(const String& path, const access_mode mode)
{
    return ::CreateFileA(c_str(path),
            mode == access_mode::read ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            0,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0);
}

template<typename String>
typename std::enable_if<
    std::is_same<typename char_type<String>::type, wchar_t>::value,
    file_handle_type
>::type open_file_helper(const String& path, const access_mode mode)
{
    return ::CreateFileW(c_str(path),
            mode == access_mode::read ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            0,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0);
}

} // win
#endif // _WIN32

/**
 * Returns the last platform specific system error (errno on POSIX and
 * GetLastError on Win) as a `std::error_code`.
 */
inline std::error_code last_error() noexcept
{
    std::error_code error;
#ifdef _WIN32
    error.assign(GetLastError(), std::system_category());
#else
    error.assign(errno, std::system_category());
#endif
    return error;
}

template<typename String>
file_handle_type open_file(const String& path, const access_mode mode,
        std::error_code& error)
{
    error.clear();
    if(detail::empty(path))
    {
        error = std::make_error_code(std::errc::invalid_argument);
        return invalid_handle;
    }
#ifdef _WIN32
    const auto handle = win::open_file_helper(path, mode);
#else // POSIX
    const auto handle = ::open(c_str(path),
            mode == access_mode::read ? O_RDONLY : O_RDWR);
#endif
    if(handle == invalid_handle)
    {
        error = detail::last_error();
    }
    return handle;
}

inline int64_t query_file_size(file_handle_type handle, std::error_code& error)
{
    error.clear();
#ifdef _WIN32
    LARGE_INTEGER file_size;
    if(::GetFileSizeEx(handle, &file_size) == 0)
    {
        error = detail::last_error();
        return 0;
    }
	return static_cast<int64_t>(file_size.QuadPart);
#else // POSIX
    struct stat sbuf;
    if(::fstat(handle, &sbuf) == -1)
    {
        error = detail::last_error();
        return 0;
    }
    return sbuf.st_size;
#endif
}

struct mmap_context
{
    char* data;
    int64_t length;
    int64_t mapped_length;
#ifdef _WIN32
    file_handle_type file_mapping_handle;
#endif
};

inline mmap_context memory_map(const file_handle_type file_handle, const int64_t offset,
    const int64_t length, const access_mode mode, std::error_code& error)
{
    const int64_t aligned_offset = make_offset_page_aligned(offset);
    const int64_t length_to_map = offset - aligned_offset + length;
#ifdef _WIN32
    const int64_t max_file_size = offset + length;
    const auto file_mapping_handle = ::CreateFileMapping(
            file_handle,
            0,
            mode == access_mode::read ? PAGE_READONLY : PAGE_READWRITE,
            win::int64_high(max_file_size),
            win::int64_low(max_file_size),
            0);
    if(file_mapping_handle == invalid_handle)
    {
        error = detail::last_error();
        return {};
    }
    char* mapping_start = static_cast<char*>(::MapViewOfFile(
            file_mapping_handle,
            mode == access_mode::read ? FILE_MAP_READ : FILE_MAP_WRITE,
            win::int64_high(aligned_offset),
            win::int64_low(aligned_offset),
            length_to_map));
    if(mapping_start == nullptr)
    {
        error = detail::last_error();
        return {};
    }
#else // POSIX
    char* mapping_start = static_cast<char*>(::mmap(
            0, // Don't give hint as to where to map.
            length_to_map,
            mode == access_mode::read ? PROT_READ : PROT_WRITE,
            MAP_SHARED,
            file_handle,
            aligned_offset));
    if(mapping_start == MAP_FAILED)
    {
        error = detail::last_error();
        return {};
    }
#endif
    mmap_context ctx;
    ctx.data = mapping_start + offset - aligned_offset;
    ctx.length = length;
    ctx.mapped_length = length_to_map;
#ifdef _WIN32
    ctx.file_mapping_handle = file_mapping_handle;
#endif
    return ctx;
}

} // namespace detail

// -- basic_mmap --

template<access_mode AccessMode, typename ByteT>
basic_mmap<AccessMode, ByteT>::~basic_mmap()
{
    conditional_sync();
    unmap();
}

template<access_mode AccessMode, typename ByteT>
basic_mmap<AccessMode, ByteT>::basic_mmap(basic_mmap&& other)
    : data_(std::move(other.data_))
    , length_(std::move(other.length_))
    , mapped_length_(std::move(other.mapped_length_))
    , file_handle_(std::move(other.file_handle_))
#ifdef _WIN32
    , file_mapping_handle_(std::move(other.file_mapping_handle_))
#endif
    , is_handle_internal_(std::move(other.is_handle_internal_))
{
    other.data_ = nullptr;
    other.length_ = other.mapped_length_ = 0;
    other.file_handle_ = invalid_handle;
#ifdef _WIN32
    other.file_mapping_handle_ = invalid_handle;
#endif
}

template<access_mode AccessMode, typename ByteT>
basic_mmap<AccessMode, ByteT>&
basic_mmap<AccessMode, ByteT>::operator=(basic_mmap&& other)
{
    if(this != &other)
    {
        // First the existing mapping needs to be removed.
        unmap();
        data_ = std::move(other.data_);
        length_ = std::move(other.length_);
        mapped_length_ = std::move(other.mapped_length_);
        file_handle_ = std::move(other.file_handle_);
#ifdef _WIN32
        file_mapping_handle_ = std::move(other.file_mapping_handle_);
#endif
        is_handle_internal_ = std::move(other.is_handle_internal_);

        // The moved from basic_mmap's fields need to be reset, because
        // otherwise other's destructor will unmap the same mapping that was
        // just moved into this.
        other.data_ = nullptr;
        other.length_ = other.mapped_length_ = 0;
        other.file_handle_ = invalid_handle;
#ifdef _WIN32
        other.file_mapping_handle_ = invalid_handle;
#endif
        other.is_handle_internal_ = false;
    }
    return *this;
}

template<access_mode AccessMode, typename ByteT>
typename basic_mmap<AccessMode, ByteT>::handle_type
basic_mmap<AccessMode, ByteT>::mapping_handle() const noexcept
{
#ifdef _WIN32
    return file_mapping_handle_;
#else
    return file_handle_;
#endif
}

template<access_mode AccessMode, typename ByteT>
template<typename String>
void basic_mmap<AccessMode, ByteT>::map(const String& path, const size_type offset,
        const size_type length, std::error_code& error)
{
    error.clear();
    if(detail::empty(path))
    {
        error = std::make_error_code(std::errc::invalid_argument);
        return;
    }
    const auto handle = detail::open_file(path, AccessMode, error);
    if(error)
    {
        return;
    }

    map(handle, offset, length, error);
    // This MUST be after the call to map, as that sets this to true.
    if(!error)
    {
        is_handle_internal_ = true;
    }
}

template<access_mode AccessMode, typename ByteT>
void basic_mmap<AccessMode, ByteT>::map(const handle_type handle,
        const size_type offset, const size_type length, std::error_code& error)
{
    error.clear();
    if(handle == invalid_handle)
    {
        error = std::make_error_code(std::errc::bad_file_descriptor);
        return;
    }

    const auto file_size = detail::query_file_size(handle, error);
    if(error)
    {
        return;
    }

    if(offset + length > file_size)
    {
        error = std::make_error_code(std::errc::invalid_argument);
        return;
    }

    const auto ctx = detail::memory_map(handle, offset,
            length == map_entire_file ? (file_size - offset) : length,
            AccessMode, error);
    if(!error)
    {
        // We must unmap the previous mapping that may have existed prior to this call.
        // Note that this must only be invoked after a new mapping has been created in
        // order to provide the strong guarantee that, should the new mapping fail, the
        // `map` function leaves this instance in a state as though the function had
        // never been invoked.
        unmap();
        file_handle_ = handle;
        is_handle_internal_ = false;
        data_ = reinterpret_cast<pointer>(ctx.data);
        length_ = ctx.length;
        mapped_length_ = ctx.mapped_length;
#ifdef _WIN32
        file_mapping_handle_ = ctx.file_mapping_handle;
#endif
    }
}

template<access_mode AccessMode, typename ByteT>
template<access_mode A>
typename std::enable_if<A == access_mode::write, void>::type
basic_mmap<AccessMode, ByteT>::sync(std::error_code& error)
{
    error.clear();
    if(!is_open())
    {
        error = std::make_error_code(std::errc::bad_file_descriptor);
        return;
    }

    if(data())
    {
#ifdef _WIN32
        if(::FlushViewOfFile(get_mapping_start(), mapped_length_) == 0
           || ::FlushFileBuffers(file_handle_) == 0)
#else // POSIX
        if(::msync(get_mapping_start(), mapped_length_, MS_SYNC) != 0)
#endif
        {
            error = detail::last_error();
            return;
        }
    }
#ifdef _WIN32
    if(::FlushFileBuffers(file_handle_) == 0)
    {
        error = detail::last_error();
    }
#endif
}

template<access_mode AccessMode, typename ByteT>
void basic_mmap<AccessMode, ByteT>::unmap()
{
    if(!is_open()) { return; }
    // TODO do we care about errors here?
#ifdef _WIN32
    if(is_mapped())
    {
        ::UnmapViewOfFile(get_mapping_start());
        ::CloseHandle(file_mapping_handle_);
    }
#else // POSIX
    if(data_) { ::munmap(const_cast<pointer>(get_mapping_start()), mapped_length_); }
#endif

    // If file_handle_ was obtained by our opening it (when map is called with a path,
    // rather than an existing file handle), we need to close it, otherwise it must not
    // be closed as it may still be used outside this instance.
    if(is_handle_internal_)
    {
#ifdef _WIN32
        ::CloseHandle(file_handle_);
#else // POSIX
        ::close(file_handle_);
#endif
    }

    // Reset fields to their default values.
    data_ = nullptr;
    length_ = mapped_length_ = 0;
    file_handle_ = invalid_handle;
#ifdef _WIN32
    file_mapping_handle_ = invalid_handle;
#endif
}

template<access_mode AccessMode, typename ByteT>
bool basic_mmap<AccessMode, ByteT>::is_mapped() const noexcept
{
#ifdef _WIN32
    return file_mapping_handle_ != invalid_handle;
#else // POSIX
    return is_open();
#endif
}

template<access_mode AccessMode, typename ByteT>
void basic_mmap<AccessMode, ByteT>::swap(basic_mmap& other)
{
    if(this != &other)
    {
        using std::swap;
        swap(data_, other.data_); 
        swap(file_handle_, other.file_handle_); 
#ifdef _WIN32
        swap(file_mapping_handle_, other.file_mapping_handle_); 
#endif
        swap(length_, other.length_); 
        swap(mapped_length_, other.mapped_length_); 
        swap(is_handle_internal_, other.is_handle_internal_); 
    }
}

template<access_mode AccessMode, typename ByteT>
template<access_mode A>
typename std::enable_if<A == access_mode::write, void>::type
basic_mmap<AccessMode, ByteT>::conditional_sync()
{
    // This is invoked from the destructor, so not much we can do about
    // failures here.
    std::error_code ec;
    sync(ec);
}

template<access_mode AccessMode, typename ByteT>
template<access_mode A>
typename std::enable_if<A == access_mode::read, void>::type
basic_mmap<AccessMode, ByteT>::conditional_sync()
{
    // noop
}

template<access_mode AccessMode, typename ByteT>
bool operator==(const basic_mmap<AccessMode, ByteT>& a,
        const basic_mmap<AccessMode, ByteT>& b)
{
    return a.data() == b.data()
        && a.size() == b.size();
}

template<access_mode AccessMode, typename ByteT>
bool operator!=(const basic_mmap<AccessMode, ByteT>& a,
        const basic_mmap<AccessMode, ByteT>& b)
{
    return !(a == b);
}

template<access_mode AccessMode, typename ByteT>
bool operator<(const basic_mmap<AccessMode, ByteT>& a,
        const basic_mmap<AccessMode, ByteT>& b)
{
    if(a.data() == b.data()) { return a.size() < b.size(); }
    return a.data() < b.data();
}

template<access_mode AccessMode, typename ByteT>
bool operator<=(const basic_mmap<AccessMode, ByteT>& a,
        const basic_mmap<AccessMode, ByteT>& b)
{
    return !(a > b);
}

template<access_mode AccessMode, typename ByteT>
bool operator>(const basic_mmap<AccessMode, ByteT>& a,
        const basic_mmap<AccessMode, ByteT>& b)
{
    if(a.data() == b.data()) { return a.size() > b.size(); }
    return a.data() > b.data();
}

template<access_mode AccessMode, typename ByteT>
bool operator>=(const basic_mmap<AccessMode, ByteT>& a,
        const basic_mmap<AccessMode, ByteT>& b)
{
    return !(a < b);
}

} // namespace mio

#endif // MIO_BASIC_MMAP_IMPL
