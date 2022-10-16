#pragma once

#include <memory>
#include <farm_ng/core/logging/expected.h>

#define FARM_ERROR_ONLY(cstr, ...) \
      ::farm_ng::Error{.details = {FARM_ERROR_DETAIL(cstr, ##__VA_ARGS__)}}


namespace pangolin
{

// Represents a Non-nullable pointer with shared ownership
// Is essentially an adapter between std::shared_ptr and farm_ng::Expected
template<class T>
class Shared{
public:
    using ExpectedT = farm_ng::Expected<Shared<T>>;
    using BadExpectedAccess = tl::bad_expected_access<farm_ng::Error>;

    /// Construct from a possibly null shared_ptr
    /// The return value is an object containing either a non-null Shared object pointer,
    //  or an farm_ng::Error object
    static ExpectedT tryFrom(std::shared_ptr<T> const& maybe_null) noexcept {
        if (!maybe_null){
            return FARM_ERROR("is null");
        }
        // For some reason, Expected seems to have trouble accepting an r-value
        // Shared<T> with its deleted copy constructor.
        Shared<T> temp(maybe_null);
        return temp; 
    }

    /// Construct and also makes interior object T
    /// The return value is an object containing either a non-null Shared object pointer,
    /// for the new object, or an farm_ng::Error object if memory allocation failst or 
    /// the constructor for the object throws
    template<class... Args>
    static ExpectedT tryMake(Args&&... args) noexcept {
        try {
          // can throw on bad memory allocation and exceptions in constructor of T.
          return tryFrom(std::make_shared<T>(std::forward<Args>(args)...));
        } catch(std::exception&e) {
            return FARM_ERROR(e.what());
        }
        return FARM_ERROR();
    }

   /// Construct from a possibly null shared_ptr
   /// Panics if shared is null. See `tryFrom()` for alternate.
   static Shared from(std::shared_ptr<T> const& shared) noexcept {
        return FARM_UNWRAP(tryFrom(shared)); 
    }

    /// Construct and also makes interior object T
    /// Panics if the constructor throws. See `tryFrom()` for alternate.
    template<class... Args>
    static Shared<T> make(Args&&... args) noexcept {
        return FARM_UNWRAP(tryMake(std::forward<Args>(args)...));
    }

    /// Returns the interior object which is guarenteed to be available
    T& operator*() { return *non_null_shared_; }

    /// Returns the interior object which is guarenteed to be available
    const T& operator*() const { return *non_null_shared_; }
    
    /// Returns the interior object which is guarenteed to be available
    T* operator->() { return non_null_shared_.get(); }

    /// Returns the interior object which is guarenteed to be available
    const T* operator->() const { return non_null_shared_.get(); }

    // Return a nullable shared_ptr<T> from this Shared<T> object
    std::shared_ptr<T> sharedPtr() const
    {
        return non_null_shared_;
    }

    // Copy constructor from derived bases
    template<std::derived_from<T> Derived> 
    Shared(const Shared<Derived>& other)
        : non_null_shared_( other.sharedPtr() )
    {
        checkMaybeThrow();
    }

    // Construct from shared_ptr
    template<std::derived_from<T> Derived> 
    Shared(const std::shared_ptr<Derived>& panic_if_null)
        : non_null_shared_(panic_if_null)
    {
        checkMaybeThrow();
    }

    // Take ownership from unique_ptr
    template<std::derived_from<T> Derived> 
    Shared(std::unique_ptr<Derived>&& panic_if_null)
        : non_null_shared_(std::move(panic_if_null))
    {
        checkMaybeThrow();
    }

    // Not sure why this is needed when the generic one
    // is defined above
    Shared(const std::shared_ptr<T>& panic_if_null)
        : non_null_shared_(panic_if_null)
    {
        checkMaybeThrow();
    }

    Shared(const Shared<T>&) = default;
    Shared(Shared<T>&&) = delete;
    Shared<T>& operator=(Shared<T>&&) = delete;
private:
    void checkMaybeThrow() const
    {
        if(!non_null_shared_) {
            throw BadExpectedAccess(FARM_ERROR_ONLY());
        }
    }

  // Class invariant:non_null_shared_ is guaranteed not to be null. 
  std::shared_ptr<T> non_null_shared_;
};

template<typename T>
using ExpectShared = farm_ng::Expected<Shared<T>>;

template<class T, class... Args>
static ExpectShared<T> tryMakeShared(Args&&... args) noexcept {
    return Shared<T>::tryMake( std::forward<Args>(args)... );
}

/// + represents a checked dereference similar to operator*()
/// Unlike Expected::operator*() which will allow for UB,
/// + will Panic if x is in the error state.
template<class T>
Shared<T>& operator+(ExpectShared<T>& x)
{
    return x.value();
}

/// + represents a checked dereference similar to operator*()
/// Unlike Expected::operator*() which will allow for UB,
/// + will Panic if x is in the error state.
template<class T>
const Shared<T>& operator+(const ExpectShared<T>& x)
{
    return x.value();
}

template<typename T>
using SharedVector = std::vector<Shared<T>>;

}