// Copyright (c) farm-ng, inc. All rights reserved.

#pragma once

#include "eigen_concepts.h"

#include <Eigen/Core>

#include <algorithm>
#include <utility>

namespace pangolin {

namespace details {

// EigenDenseType may be a map or view or abstract base class or something.
// This is an alias for the corresponding concrete type with storage
template <EigenDenseType TT>
using EigenConcreteType =
    std::remove_reference_t<decltype(std::declval<TT>().eval())>;

template <class TScalar>
class Min {
 public:
  static TScalar impl(TScalar const& lhs, TScalar const& rhs) {
    return std::min(lhs, rhs);
  }
};

template <EigenDenseType TT>
class Min<TT> {
 public:
  static EigenConcreteType<TT> impl(TT const& lhs, TT const& rhs) {
    return lhs.cwiseMin(rhs);
  }
};

template <class TScalar>
class Max {
 public:
  static TScalar impl(TScalar const& lhs, TScalar const& rhs) {
    return std::max(lhs, rhs);
  }
};

template <EigenDenseType TT>
class Max<TT> {
 public:
  static EigenConcreteType<TT> impl(TT const& lhs, TT const& rhs) {
    return lhs.cwiseMax(rhs);
  }
};

template <class TScalar>
class Cast {
 public:
  template <typename To>
  static To impl(TScalar const& s) {
    return static_cast<To>(s);
  }
};

template <EigenType TT>
class Cast<TT> {
 public:
  template <typename To>
  static auto impl(TT const& v) {
    return v.template cast<typename To::Scalar>();
  }
};

template <class TScalar>
class Eval {
 public:
  static TScalar impl(TScalar const& s) { return s; }
};

template <EigenType TT>
class Eval<TT> {
 public:
  static auto impl(TT const& v) { return v.eval(); }
};

template <class TScalar>
class AllTrue {
 public:
  static bool impl(TScalar const& s) { return bool(s); }
};

template <EigenDenseType TT>
class AllTrue<TT> {
 public:
  static bool impl(TT const& v) { return v.all(); }
};

template <class TScalar>
class AnyTrue {
 public:
  static bool impl(TScalar const& s) { return bool(s); }
};

template <EigenDenseType TT>
class AnyTrue<TT> {
 public:
  static bool impl(TT const& v) { return v.any(); }
};

template <class TScalar>
class IsFinite {
 public:
  static bool impl(TScalar const& s) { return std::isfinite(s); }
};

template <EigenDenseType TT>
class IsFinite<TT> {
 public:
  static bool impl(TT const& v) { return v.isFinite().all(); }
};

template <class TScalar>
class Reduce {
 public:
  using Aggregate = TScalar;

  template <class TReduce, class Func>
  static void impl_unary(TScalar const& s, TReduce& reduce, Func const& f) {
    f(s, reduce);
  }

  template <class TReduce, class Func>
  static void impl_binary(
      TScalar const& a, TScalar const& b, TReduce& reduce, Func const& f) {
    f(a, b, reduce);
  }
};

template <EigenDenseType TT>
class Reduce<TT> {
 public:
  template <class TReduce, class Func>
  static void impl_unary(TT const& v, TReduce& reduce, Func const& f) {
    for (int r = 0; r < v.rows(); ++r) {
      for (int c = 0; c < v.cols(); ++c) {
        f(v(r, c), reduce);
      }
    }
  }

  template <class TReduce, class Func>
  static void impl_binary(
      TT const& a, TT const& b, TReduce& reduce, Func const& f) {
    for (int r = 0; r < a.rows(); ++r) {
      for (int c = 0; c < a.cols(); ++c) {
        f(a(r, c), b(r, c), reduce);
      }
    }
  }
};

}  // namespace details

template <class To, class TT>
auto cast(const TT& p) {
  return details::Cast<TT>::template impl<To>(p);
}

template <class TT>
auto eval(const TT& p) {
  return details::Eval<TT>::impl(p);
}

template <class TT>
bool allTrue(const TT& p) {
  return details::AllTrue<TT>::impl(p);
}

template <class TT>
bool anyTrue(const TT& p) {
  return details::AnyTrue<TT>::impl(p);
}

template <class TT>
bool isFinite(const TT& p) {
  return details::IsFinite<TT>::impl(p);
}

template <class TT>
TT min(const TT& a, const TT& b) {
  return details::Min<TT>::impl(a, b);
}

template <class TT>
TT max(const TT& a, const TT& b) {
  return details::Max<TT>::impl(a, b);
}

template <class TT, class TFunc, class TReduce>
void reduceArg(const TT& x, TReduce& reduce, TFunc&& func) {
  details::Reduce<TT>::impl_unary(x, reduce, std::forward<TFunc>(func));
}

template <class TT, class TFunc, class TReduce>
void reduceArg(const TT& a, const TT& b, TReduce& reduce, TFunc&& func) {
  details::Reduce<TT>::impl_binary(a, b, reduce, std::forward<TFunc>(func));
}

template <class TT, class TFunc, class TReduce>
TReduce reduce(const TT& x, TReduce const& initial, TFunc&& func) {
  TReduce reduce = initial;
  details::Reduce<TT>::impl_unary(x, reduce, std::forward<TFunc>(func));
  return reduce;
}

template <class TT, class TFunc, class TReduce>
TReduce reduce(const TT& a, const TT& b, TReduce const& initial, TFunc&& func) {
  TReduce reduce = initial;
  details::Reduce<TT>::impl_binary(a, b, reduce, std::forward<TFunc>(func));
  return reduce;
}

}  // namespace pangolin
