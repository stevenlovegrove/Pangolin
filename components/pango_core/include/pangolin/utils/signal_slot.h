#pragma once

#include <signals/signals.hpp>

namespace pangolin {

template<typename ... A>
using Signal= fteng::signal<A...>;

using Connection = fteng::connection;

}
