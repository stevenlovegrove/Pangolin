#pragma once

#include <array>

namespace pangolin
{

constexpr int kDynamic = -1;
constexpr int kMaxMultiDimSizeBytes = 64;

template<int ...kSizes>
struct MultiDim;

template<int kSize>
struct MultiDim<kSize>
{
    static constexpr int kDim = 1;
    static constexpr int kDynamicDim =
        [](){if constexpr(kSize==kDynamic) return 1; else return 0; }();
    using Index = std::array<int,kDim>;
    using DynamicSize = std::array<int,kDynamicDim>;
    static constexpr int kFixedVolume =
        [](){if constexpr(kSize!=kDynamic) return kSize; else return 0; }();


    DynamicSize sizes;
};

template<int kSizes0, int ...kSizes>
struct MultiDim<kSizes0, kSizes...>
{
    static constexpr int kDim = 1 + sizeof...(kSizes);
    static constexpr int kDynamicDim =
        MultiDim<kSizes0>::kDynamicDim + MultiDim<kSizes...>::kDynamicDim;
    using Index = std::array<int,kDim>;
    using DynamicSize = std::array<int,kDynamicDim>;
    static constexpr int kFixedVolume =
        MultiDim<kSizes0>::kFixedVolume + MultiDim<kSizes...>::kFixedVolume;

    DynamicSize sizes;
};

template<typename T, int kStaticSize, bool kStatic>
struct MaybeStaticStorage;

template<typename T, int kStaticSize>
struct MaybeStaticStorage<T,kStaticSize,false>
{
    static constexpr int kSizeAtCompileTime = kDynamic;

    MaybeStaticStorage(int size)
        : data(new T[size])
    {
    }

    int size() const {
        return kSizeAtCompileTime;
    }

    std::unique_ptr<T[]> data;
};

template<typename T, int kStaticSize>
struct MaybeStaticStorage<T,kStaticSize,true>
{
    static constexpr int kSizeAtCompileTime = kStaticSize;

    MaybeStaticStorage(int size) {
        assert(kStaticSize == size);
    }

    int size() const {
        return data.size();
    }

    std::array<T,kStaticSize> data;
};

template<int ...kSizes>
struct MultiDimAccessDims{};

// Tptr could be raw, unique, shared etc
template<class T, class Tptr, int ...kSizes>
struct MultiDimArray
{
    using Dim = MultiDim<kSizes...>;
    static constexpr bool kPackIntoStruct =
        Dim::kDynamicDim==0 && (sizeof(T)* Dim::kFixedVolume <= kMaxMultiDimSizeBytes);

    using Storage = MaybeStaticStorage<T, Dim::kFixedVolume, kPackIntoStruct>;

    typename Dim::DynamicSize dynamic_dim_sizes;
    Tptr data;
};

// Tptr could be raw, unique, shared etc
template<class T, class Tptr, int ...kSizes>
struct MultiDimView
{
    using Dim = MultiDim<kSizes...>;

    typename Dim::DynamicSize dynamic_dim_sizes;
    // typename Dim::DynamicSize dynamic_dim_pitch;
    Tptr data;
};


}
