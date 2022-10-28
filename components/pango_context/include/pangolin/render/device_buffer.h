#include <pangolin/utils/shared.h>
#include <pangolin/maths/multi_dim.h>
#include <sophus/image/image.h>

namespace pangolin
{

struct DeviceBuffer
{
    static constexpr size_t kMaxDims = 4;

    enum class UseHint
    {
        Generic,
        VertexIndices,
        VertexAttributes,
        PixelData
    };

    virtual ~DeviceBuffer() {}

    ////////////////////////////////////
    // Array-like access


    ////////////////////////////////////
    // Image-like access

    struct UpdateParams
    {
        // What are the sizes of each dimension.
        // Empty dimensions should be 0. An image with C color channels
        // of width*height in row major format would be [C,width,height,0]
        // for example
        std::array<size_t,kMaxDims> sizes = {};

        // What are the offsets in each dimension for this update
        std::array<size_t,kMaxDims> offsets = {};

    };
    // void update(ImageView) = 0;


    struct Params {
        UseHint use_hint;
    };
    static Shared<DeviceBuffer> Create(const Params& p);

    static Shared<DeviceBuffer> TakeFrom();
};


}
