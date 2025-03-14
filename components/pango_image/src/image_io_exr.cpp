#include <pangolin/platform.h>

#include <cstdint>
#include <fstream>
#include <pangolin/image/typed_image.h>

#ifdef HAVE_OPENEXR
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfIO.h>
#endif // HAVE_OPENEXR

namespace pangolin {

#ifdef HAVE_OPENEXR
Imf::PixelType OpenEXRPixelType(int channel_bits)
{
    if( channel_bits == 16 ) {
        return Imf::PixelType::HALF;
    }else if( channel_bits == 32 ) {
        return Imf::PixelType::FLOAT;
    }else{
        throw std::runtime_error("Unsupported OpenEXR Pixel Type.");
    }
}

void SetOpenEXRChannels(Imf::ChannelList& ch, const pangolin::PixelFormat& fmt)
{
    const char* CHANNEL_NAMES[] = {"R","G","B","A"};
    for(size_t c=0; c < fmt.channels; ++c) {
        ch.insert( CHANNEL_NAMES[c], Imf::Channel(OpenEXRPixelType(fmt.channel_bits[c])) );
    }
}

class StdIStream: public Imf::IStream
{
  public:
    StdIStream (std::istream &is):
        Imf::IStream ("stream"),
        _is (&is)
    {
    }

    virtual bool read (char c[/*n*/], int n)
    {
        if (!*_is)
            throw std::runtime_error("Unexpected end of file.");
        _is->read (c, n);
        if (_is->gcount() < n)
        {
            throw std::runtime_error("Early end of file");
            return false;
        }
        return true;
    }

    virtual uint64_t tellg ()
    {
        return std::streamoff (_is->tellg());
    }

    virtual void seekg (uint64_t pos)
    {
        _is->seekg (pos);
    }

    virtual void clear ()
    {
        _is->clear();
    }

  private:
    std::istream *	_is;
};

PixelFormat GetPixelFormat(const Imf::Header& header)
{
    const Imf::ChannelList &channels = header.channels();
    size_t count = 0;

    std::stringstream pixelFormat;
    size_t depth = 0;
    for (Imf::ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i){
        const Imf::Channel& channel = i.channel();
        if(depth == 0)
        {
          switch(channel.type)
          {
            case Imf::FLOAT: depth = 32; break;
            case Imf::HALF: depth = 16; break;
            default:
              throw std::invalid_argument("Currently, only floating-point OpenEXR files are supported.");
          }
        }
        count += 1;
        pixelFormat << i.name();
    }

    std::string colors = pixelFormat.str();
    if (colors == "Y" || colors == "R") {
      colors = "GRAY";
    }

    if ((count == 1 && colors != "GRAY")
        || (count == 3 && colors != "RGB" && colors != "BGR")
        || (count == 4 && colors != "RGBA" && colors != "ABGR")) {
      throw std::runtime_error("bad color format");
    }
    if (count != 1 && count != 3 && count != 4) {
      throw std::invalid_argument("Currently, only 1, 3 or 4-channel OpenEXR files are supported.");
    }

    return PixelFormatFromString(colors + std::to_string(depth*count) + "F");
}

#endif //HAVE_OPENEXR

TypedImage LoadExr(std::istream& source)
{
#ifdef HAVE_OPENEXR
    StdIStream istream(source);
    Imf::InputFile file(istream);
    PANGO_WARNING(file.isComplete());

    Imath::Box2i dw = file.header().dataWindow();
    int width = dw.max.x - dw.min.x + 1;
    int height = dw.max.y - dw.min.y + 1;

    PixelFormat format = GetPixelFormat(file.header());
    TypedImage img(width, height, format);

    char *imgBase = (char *) img.ptr - (dw.min.x + dw.min.y * width) * sizeof(float) * format.channels;
    Imf::FrameBuffer fb;

    const Imf::ChannelList &channels = file.header().channels();
    size_t c = 0;
    unsigned int d = format.channel_bit_depth;
    Imf::PixelType pixeltype = Imf::PixelType::UINT;
    switch(d)
    {
      case 16: pixeltype = Imf::HALF; break;
      case 32: pixeltype = Imf::FLOAT; break;
      throw std::invalid_argument("Currently, only floating-point OpenEXR files are supported.");
    }
    unsigned int pixelsize = d/8;

    auto writeChannel = [&](Imf::ChannelList::ConstIterator& i, size_t j) {
      fb.insert(i.name(), Imf::Slice(
                      pixeltype, imgBase + pixelsize * j,
                      pixelsize * format.channels,
                      pixelsize * format.channels * size_t(width)));
    };
    if(format.format == "ABGR128F") {
      format.format = "RGBA128F";
      static std::map<std::string, size_t> channelOrder = {
        {"R", 0}, {"G", 1}, {"B", 2}, {"A", 3}
      };
      for (Imf::ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i) {
        writeChannel(i, channelOrder[i.name()]);
      }
    }
    else {
      for (Imf::ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i) {
        writeChannel(i, c++);
      }
    }

    file.setFrameBuffer(fb);
    file.readPixels(dw.min.y, dw.max.y);

    return img;
#else
    PANGOLIN_UNUSED(source);
    throw std::runtime_error("Rebuild Pangolin for EXR support.");
#endif //HAVE_OPENEXR
}

void SaveExr(const Image<unsigned char>& image_in, const pangolin::PixelFormat& fmt, const std::string& filename, bool top_line_first)
{
#ifdef HAVE_OPENEXR
    ManagedImage<unsigned char> flip_image;
    Image<unsigned char> image;

    if(top_line_first) {
        image = image_in;
    }else{
        flip_image.Reinitialise(image_in.w, image_in.h, image_in.pitch);
        for(size_t y=0; y<image_in.h; ++y) {
            std::memcpy(flip_image.RowPtr(y), image_in.RowPtr(image_in.h - 1 - y), image_in.pitch);
        }
        image = flip_image;
    }


    Imf::Header header (image.w, image.h);
    SetOpenEXRChannels(header.channels(), fmt);

    Imf::OutputFile file (filename.c_str(), header);
    Imf::FrameBuffer frameBuffer;

    size_t ch_bits = 0;
    const char* CHANNEL_NAMES[] = {"R","G","B","A"};
    for(unsigned int i=0; i<fmt.channels; i++)
    {
        const Imf::Channel *channel = header.channels().findChannel(CHANNEL_NAMES[i]);
        frameBuffer.insert(
            CHANNEL_NAMES[i],
            Imf::Slice(
                channel->type,
                (char*)image.ptr + (ch_bits/8),
                fmt.bpp/8,  // xstride
                image.pitch // ystride
            )
        );

        ch_bits += fmt.channel_bits[i];
    }

    file.setFrameBuffer(frameBuffer);
    file.writePixels(image.h);

#else
    PANGOLIN_UNUSED(image_in);
    PANGOLIN_UNUSED(fmt);
    PANGOLIN_UNUSED(filename);
    PANGOLIN_UNUSED(top_line_first);
    throw std::runtime_error("EXR Support not enabled. Please rebuild Pangolin.");
#endif // HAVE_OPENEXR
}

}
