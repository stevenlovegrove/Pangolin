#include <pangolin/render/conventions.h>

namespace pangolin
{

Conventions& Conventions::global()
{
  static Conventions instance = {};
  return instance;
}

}  // namespace pangolin
