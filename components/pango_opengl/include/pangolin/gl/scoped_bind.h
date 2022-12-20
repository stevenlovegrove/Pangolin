#pragma once

#include <functional>

namespace pangolin
{

// Need specialization like this for every type (in a CPP)
// template<>
// thread_local ScopedBind<MyType>* ScopedBind<MyType>::current = nullptr;

// RAII binding of variables. Specialized for a particular resource
template <typename T>  // T really just defines the domain 'parent'
struct [[nodiscard]] ScopedBind {
  // Not copyable
  ScopedBind(const ScopedBind&) = delete;

  // but moveable
  ScopedBind(ScopedBind&& o) { *this = std::move(o); }
  ScopedBind& operator=(ScopedBind&& o)
  {
    parent_ = o.parent_;
    bind_func_ = o.bind_func_;
    unbind_func_ = o.unbind_func_;
    o.unbind_func_ = std::nullopt;
    o.parent_ = nullptr;
    return *this;
  }

  ScopedBind(
      const std::function<void()>& bind_func,
      const std::function<void()>& unbind_func) :
      bind_func_(bind_func),
      unbind_func_(unbind_func),
      parent_(getLocalActiveScopePtr())
  {
    getLocalActiveScopePtr() = this;
    bind_func();
  }

  ~ScopedBind()
  {
    if (parent_) {
      parent_->bind_func_();
      getLocalActiveScopePtr() = parent_;
    } else if (unbind_func_) {
      (*unbind_func_)();
      getLocalActiveScopePtr() = nullptr;
    }
  }

  private:
  using pScopedBind = ScopedBind*;

  // thread local singleton inside so no race conditions
  static pScopedBind& getLocalActiveScopePtr();

  std::function<void()> bind_func_;
  std::optional<std::function<void()>> unbind_func_;
  ScopedBind* parent_;
};

}  // namespace pangolin
