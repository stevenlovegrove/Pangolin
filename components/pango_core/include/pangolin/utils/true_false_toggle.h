#pragma once

namespace pangolin {

enum class TrueFalseToggle
{
    False=0,
    True=1,
    Toggle=2
};

inline bool to_bool(const TrueFalseToggle on_off_toggle, const bool current_value)
{
    switch (on_off_toggle) {
    case TrueFalseToggle::True: return true;
    case TrueFalseToggle::False: return false;
    case TrueFalseToggle::Toggle: return !current_value;
    default: return false;
    }
}

inline bool should_toggle(const TrueFalseToggle on_off_toggle, const bool current_value)
{
    return ( (on_off_toggle == TrueFalseToggle::Toggle) ||
             (current_value != (on_off_toggle==TrueFalseToggle::True))
           );
}

}
