#pragma once

#include <streambuf>
#include <vector>

namespace pangolin
{

// A simple streambuf wrapper around std::vector for memory buffer use
struct memstreambuf : public std::streambuf {
  public:
  memstreambuf(size_t initial_buffer_size)
  {
    buffer.reserve(initial_buffer_size);
  }

  // Avoiding use of std::streambuf's move constructor, since it is missing for
  // old GCC
  memstreambuf(memstreambuf&& o) : buffer(std::move(o.buffer))
  {
    pubseekpos(o.pubseekoff(0, std::ios_base::cur));
  }

  size_t size() const { return buffer.size(); }

  const unsigned char* data() const { return buffer.data(); }

  void clear() { buffer.clear(); }

  std::vector<unsigned char> buffer;

  protected:
  std::streamsize xsputn(const char_type* __s, std::streamsize __n) override
  {
    buffer.insert(buffer.end(), __s, __s + __n);
    return __n;
  }

  int_type overflow(int_type __c) override
  {
    buffer.push_back(static_cast<unsigned char>(__c));
    return __c;
  }
};

class StreambufMemoryWrapper : public std::basic_streambuf<char>
{
  public:
  StreambufMemoryWrapper(const uint8_t* p, size_t l)
  {
    setg((char*)p, (char*)p, (char*)p + l);
  }
};

class IStreamMemoryWrapper : public std::istream
{
  public:
  IStreamMemoryWrapper(const uint8_t* p, size_t l) :
      std::istream(&_buffer), _buffer(p, l)
  {
    rdbuf(&_buffer);
  }

  private:
  StreambufMemoryWrapper _buffer;
};

}  // namespace pangolin
