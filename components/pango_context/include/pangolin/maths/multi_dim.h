#pragma once

#include <array>

namespace pangolin
{

constexpr int kDynamic = -1;
constexpr int kMaxMultiDimSizeBytes = 128;

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
        MultiDim<kSizes0>::kFixedVolume * MultiDim<kSizes...>::kFixedVolume;

    DynamicSize sizes;
};

template<typename T, int kStaticSize, bool kStatic>
struct MaybeStaticStorage;

template<typename T, int kStaticSize>
struct MaybeStaticStorage<T,kStaticSize,false>
{
    static constexpr int kSizeAtCompileTime = kDynamic;

    MaybeStaticStorage() = default;
    MaybeStaticStorage(MaybeStaticStorage&&) = default;
    MaybeStaticStorage(const MaybeStaticStorage&) = delete;
    MaybeStaticStorage& operator=(MaybeStaticStorage&&) = default;

    MaybeStaticStorage(int size)
        : data(new T[size])
    {
    }

    constexpr int size() const {
        return kSizeAtCompileTime;
    }

    std::unique_ptr<T[]> data;
};

template<typename T, int kStaticSize>
struct MaybeStaticStorage<T,kStaticSize,true>
{
    static constexpr int kSizeAtCompileTime = kStaticSize;

    MaybeStaticStorage() = default;
    MaybeStaticStorage(MaybeStaticStorage&&) = default;
    MaybeStaticStorage(const MaybeStaticStorage&) = delete;
    MaybeStaticStorage& operator=(MaybeStaticStorage&&) = default;

    MaybeStaticStorage(int size) {
        assert(kStaticSize == size);
    }

    constexpr int size() const {
        return data.size();
    }

    std::array<T,kStaticSize> data;
};

template<int ...kSizes>
struct MultiDimAccessDims{};

template<class... Xs>
struct AllAre;

template<class T, class X, class... Xs>
requires (std::is_same<T,X>::value)
struct AllAre<T,X,Xs...> : AllAre<T,Xs...> {};

template<class T>
struct AllAre<T> : std::true_type {};

template<class T, int N>
T arrayProduct(const std::array<T,N>& arr)
{
    T sum = static_cast<T>(0);
    for(T v : arr) {
        sum += v;
    }
    return sum;
}


template<class T, int ...kSizes>
struct MultiDimArray
{
    using Dim = MultiDim<kSizes...>;
    using DynamicSize = typename Dim::DynamicSize;
    using Index = typename Dim::Index;
    static constexpr bool kPackIntoStruct =
        Dim::kDynamicDim==0 && (sizeof(T)* Dim::kFixedVolume <= kMaxMultiDimSizeBytes);
    using Storage = MaybeStaticStorage<T, Dim::kFixedVolume, kPackIntoStruct>;

    MultiDimArray()
    {
        static_assert(Dim::kDynamicDim == 0);
    }

    MultiDimArray(const DynamicSize& dynamic_sizes)
    : dynamic_dim_sizes(dynamic_sizes),
      data(Dim::kFixedVolume * arrayProduct(dynamic_sizes))
    {
    }

    T& operator()(const Index& idx)
    {
        // ...
    }

    DynamicSize dynamic_dim_sizes;
    Storage data;
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
