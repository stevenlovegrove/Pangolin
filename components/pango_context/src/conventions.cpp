#include <pangolin/maths/conventions.h>

namespace pangolin {

Conventions& Conventions::global() {
  static Conventions instance = {};
  return instance;
}

}  // namespace pangolin
