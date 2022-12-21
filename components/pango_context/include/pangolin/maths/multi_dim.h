#pragma once

#include <array>
#include <memory>

namespace pangolin
{

int constexpr kDynamic = -1;
int constexpr kMaxMultiDimSizeBytes = 128;

template <int... kSizes>
struct MultiDim;

template <int kSize>
struct MultiDim<kSize> {
  static int constexpr kDim = 1;
  static int constexpr kDynamicDim = []() {
    if constexpr (kSize == kDynamic)
      return 1;
    else
      return 0;
  }();
  using Index = std::array<int, kDim>;
  using DynamicSize = std::array<int, kDynamicDim>;
  static int constexpr kFixedVolume = []() {
    if constexpr (kSize != kDynamic)
      return kSize;
    else
      return 0;
  }();

  DynamicSize sizes;
};

template <int kSizes0, int... kSizes>
struct MultiDim<kSizes0, kSizes...> {
  static int constexpr kDim = 1 + sizeof...(kSizes);
  static int constexpr kDynamicDim =
      MultiDim<kSizes0>::kDynamicDim + MultiDim<kSizes...>::kDynamicDim;
  using Index = std::array<int, kDim>;
  using DynamicSize = std::array<int, kDynamicDim>;
  static int constexpr kFixedVolume =
      MultiDim<kSizes0>::kFixedVolume * MultiDim<kSizes...>::kFixedVolume;

  DynamicSize sizes;
};

template <typename T, int kStaticSize, bool kStatic>
struct MaybeStaticStorage;

template <typename T, int kStaticSize>
struct MaybeStaticStorage<T, kStaticSize, false> {
  static int constexpr kSizeAtCompileTime = kDynamic;

  MaybeStaticStorage() = default;
  MaybeStaticStorage(MaybeStaticStorage&&) = default;
  MaybeStaticStorage(MaybeStaticStorage const&) = delete;
  MaybeStaticStorage& operator=(MaybeStaticStorage&&) = default;

  MaybeStaticStorage(int size) : data(new T[size]) {}

  int constexpr size() const { return kSizeAtCompileTime; }

  std::unique_ptr<T[]> data;
};

template <typename T, int kStaticSize>
struct MaybeStaticStorage<T, kStaticSize, true> {
  static int constexpr kSizeAtCompileTime = kStaticSize;

  MaybeStaticStorage() = default;
  MaybeStaticStorage(MaybeStaticStorage&&) = default;
  MaybeStaticStorage(MaybeStaticStorage const&) = delete;
  MaybeStaticStorage& operator=(MaybeStaticStorage&&) = default;

  MaybeStaticStorage(int size)
  {
    // assert(kStaticSize == size);
  }

  int constexpr size() const { return data.size(); }

  std::array<T, kStaticSize> data;
};

template <int... kSizes>
struct MultiDimAccessDims {
};

template <class... Xs>
struct AllAre;

template <class T, class X, class... Xs>
requires(std::is_same<T, X>::value) struct AllAre<T, X, Xs...>
    : AllAre<T, Xs...> {
};

template <class T>
struct AllAre<T> : std::true_type {
};

template <class T, int N>
T arrayProduct(std::array<T, N> const& arr)
{
  T sum = static_cast<T>(0);
  for (T v : arr) {
    sum += v;
  }
  return sum;
}

template <class T, int... kSizes>
struct MultiDimArray {
  using Dim = MultiDim<kSizes...>;
  using DynamicSize = typename Dim::DynamicSize;
  using Index = typename Dim::Index;
  static bool constexpr kPackIntoStruct =
      Dim::kDynamicDim == 0 &&
      (sizeof(T) * Dim::kFixedVolume <= kMaxMultiDimSizeBytes);
  using Storage = MaybeStaticStorage<T, Dim::kFixedVolume, kPackIntoStruct>;

  MultiDimArray() { static_assert(Dim::kDynamicDim == 0); }

  MultiDimArray(DynamicSize const& dynamic_sizes) :
      dynamic_dim_sizes(dynamic_sizes),
      data(Dim::kFixedVolume * arrayProduct(dynamic_sizes))
  {
  }

  T& operator()(Index const& idx)
  {
    // ...
  }

  DynamicSize dynamic_dim_sizes;
  Storage data;
};

// Tptr could be raw, unique, shared etc
template <class T, class Tptr, int... kSizes>
struct MultiDimView {
  using Dim = MultiDim<kSizes...>;

  typename Dim::DynamicSize dynamic_dim_sizes;
  // typename Dim::DynamicSize dynamic_dim_pitch;
  Tptr data;
};

}  // namespace pangolin
