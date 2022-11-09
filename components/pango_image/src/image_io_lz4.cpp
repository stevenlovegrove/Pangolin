#include <fstream>
#include <memory>

#include <pangolin/image/runtime_image.h>

#ifdef HAVE_LZ4
#  include <lz4.h>
#endif

namespace pangolin {

#pragma pack(push, 1)
struct lz4_image_header
{
    char magic[3];
    char fmt[16];
    size_t w, h;
    int64_t compressed_size;
};
#pragma pack(pop)

void SaveLz4(const IntensityImage<>& image, std::ostream& out, int compression_level)
{
#ifdef HAVE_LZ4
    const int64_t src_size = image.sizeBytes();
    const int64_t max_dst_size = LZ4_compressBound(src_size);
    std::unique_ptr<char[]> output_buffer(new char[max_dst_size]);

    // Same as LZ4_compress_default(), but allows to select an "acceleration" factor.
    // The larger the acceleration value, the faster the algorithm, but also the lesser the compression.
    // It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed.
    // An acceleration value of "1" is the same as regular LZ4_compress_default()
    // Values <= 0 will be replaced by ACCELERATION_DEFAULT (see lz4.c), which is 1.
    const int64_t compressed_data_size = LZ4_compress_fast((char*)image.rawPtr(), output_buffer.get(), src_size, max_dst_size, compression_level);

    if (compressed_data_size < 0)
        throw std::runtime_error("A negative result from LZ4_compress_default indicates a failure trying to compress the data.");
    if (compressed_data_size == 0)
        throw std::runtime_error("A result of 0 for LZ4 means compression worked, but was stopped because the destination buffer couldn't hold all the information.");

    lz4_image_header header;
    memcpy(header.magic,"LZ4",3);
    strncpy(header.fmt, ToString(image.pixelType()).c_str(), sizeof(header.fmt));
    header.w = image.width();
    header.h = image.height();
    header.compressed_size = compressed_data_size;
    out.write((char*)&header, sizeof(header));

    out.write(output_buffer.get(), compressed_data_size);

#else
    PANGOLIN_UNUSED(image);
    PANGOLIN_UNUSED(fmt);
    PANGOLIN_UNUSED(out);
    PANGOLIN_UNUSED(compression_level);
    throw std::runtime_error("Rebuild Pangolin for LZ4 support.");
#endif // HAVE_LZ4
}

IntensityImage<> LoadLz4(std::istream& in)
{
#ifdef HAVE_LZ4
    // Read in header, uncompressed
    lz4_image_header header;
    in.read( (char*)&header, sizeof(header));

    IntensityImage<> img(ImageSize(header.w, header.h), PixelFormatFromString(header.fmt));
    std::unique_ptr<char[]> input_buffer(new char[header.compressed_size]);

    in.read(input_buffer.get(), header.compressed_size);
    const int decompressed_size = LZ4_decompress_safe(input_buffer.get(), (char*)const_cast<uint8_t*>(img.rawPtr()), header.compressed_size, img.sizeBytes());

    PANGO_THROW_IF(decompressed_size < 0,
        "LZ4_decompress_safe exited with code ({}).", decompressed_size);
    PANGO_THROW_IF(decompressed_size == 0,
        "Documentation in lz4.h says this should neve happen.");
    PANGO_THROW_IF(decompressed_size != (int)img.sizeBytes(),
        "decompressed size {} is not equal to predicted size {}", decompressed_size, img.sizeBytes());

    return img;
#else
    PANGOLIN_UNUSED(in);
    throw std::runtime_error("Rebuild Pangolin for LZ4 support.");
#endif // HAVE_LZ4
}

}
