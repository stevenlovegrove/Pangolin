#pragma once

#include <Eigen/Core>
#include <sophus2/concepts/point.h>

namespace pangolin
{

template <typename TT>
concept FloatingPointType = std::is_floating_point_v<TT>;

template <typename TT>
concept EigenOrFloatType =
    sophus2::concepts::EigenType<TT> || FloatingPointType<TT>;

template <typename Ts>
struct DimTraits;

template <FloatingPointType TT>
struct DimTraits<TT> {
  constexpr static int kNumRows = 1;
  constexpr static int kNumCols = 1;
};

template <sophus2::concepts::EigenType TT>
struct DimTraits<TT> {
  constexpr static int kNumRows = TT::RowsAtCompileTime;
  constexpr static int kNumCols = TT::ColsAtCompileTime;
};

template <EigenOrFloatType... Ts>
struct VstackTraits {
  using HeadT = typename std::tuple_element_t<0, std::tuple<Ts...>>;
  using Scalar = typename HeadT::Scalar;

  constexpr static bool kDynamicRows =
      ((DimTraits<Ts>::kNumRows == Eigen::Dynamic) || ...);
  constexpr static bool kDynamicCols =
      ((DimTraits<Ts>::kNumCols == Eigen::Dynamic) || ...);
  constexpr static int kNumRows =
      kDynamicRows ? Eigen::Dynamic : (DimTraits<Ts>::kNumRows + ...);
  constexpr static int kNumCols =
      kDynamicCols ? Eigen::Dynamic : DimTraits<HeadT>::kNumCols;

  static_assert(
      ((DimTraits<Ts>::kNumCols == Eigen::Dynamic ||
        DimTraits<Ts>::kNumCols == kNumCols) &&
       ...));

  // Little more work to support dynamic dimensions.
  static_assert(!kDynamicRows && !kDynamicCols);

  using DenseMatrixType = Eigen::Matrix<Scalar, kNumRows, kNumCols>;
};

template <EigenOrFloatType T, EigenOrFloatType... Ts>
auto vstack(const T& t, const Ts&... ts)
{
  typename VstackTraits<T, Ts...>::DenseMatrixType out;
  ((out << t), ..., ts);
  return out;
}

}  // namespace pangolin
