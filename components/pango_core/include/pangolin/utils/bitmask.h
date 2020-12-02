// Modified version of https://gpfault.net/posts/typesafe-bitmasks.txt.html (public domain)

#include <type_traits>

namespace pangolin {

template <class option_type,
          bool already_pow2,
          // The line below ensures that bitmask can only be used with enums.
          typename = typename std::enable_if<std::is_enum<option_type>::value>::type>
class bitmask {
  // The type we'll use for storing the value of our bitmask should be the same
  // as the enum's underlying type.
  using underlying_type = typename std::underlying_type<option_type>::type;

  // This method helps us avoid having to explicitly set enum values to powers
  // of two.
  static constexpr underlying_type mask_value(option_type o) {

    return already_pow2 ? o : 1 << static_cast<underlying_type>(o);
  }

  // Private ctor to be used internally.
  explicit constexpr bitmask(underlying_type o) : mask_(o) {}

public:
  // Default ctor creates a bitmask with no options selected.
  constexpr bitmask() : mask_(0) {}

  // Creates a bitmask with just one bit set.
  // This ctor is intentionally non-explicit, to allow for stuff like:
  // FunctionExpectingBitmask(Options::Opt1)
  constexpr bitmask(option_type o) : mask_(mask_value(o)) {}

  // Set the bit corresponding to the given option.
  constexpr bitmask operator|(option_type t) {
    return bitmask(mask_ | (mask_value(t)));
  }

  // Set the bit corresponding to the given option.
  void operator|=(option_type t) {
    mask_ |= mask_value(t);
  }

  // Get the value of the bit corresponding to the given option.
  constexpr bool operator&(option_type t) {
    return mask_ & mask_value(t);
  }

  // Set the bit corresponding to the given option.
  void operator&=(option_type t) {
    mask_ &= mask_value(t);
  }

  underlying_type mask()
  {
      return mask_;
  }

  void set(option_type t, bool on)
  {
      if(on) {
          mask_ |= mask_value(t);
      }else{
          mask_ &= ~mask_value(t);
      }
  }

private:
    underlying_type mask_ = 0;
};

}
