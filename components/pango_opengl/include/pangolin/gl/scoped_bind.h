#pragma once

namespace pangolin
{

// Need specialization like this for every type (in a CPP)
// template<>
// thread_local ScopedBind<MyType>* ScopedBind<MyType>::current = nullptr;


// RAII binding of variables. Specialized for a particular resource
template<typename T> // T really just defines the domain 'parent'
struct [[nodiscard]] ScopedBind
{
    // Not copyable
    ScopedBind(const ScopedBind&) = delete;

    ScopedBind(
        const std::function<void()>& bind_func,
        const std::function<void()>& unbind_func
    ): bind_func_(bind_func), unbind_func_(unbind_func), parent_(current)
    {
        bind_func();
    }

    ~ScopedBind()
    {
        if(parent_) {
            parent_->bind_func_();
        }else{
            unbind_func_();
        }
    }

private:
    // thread local so no race conditions
    static thread_local ScopedBind* current;

    std::function<void()> bind_func_;
    std::function<void()> unbind_func_;
    ScopedBind* parent_;
};

}
