#include <pangolin/platform.h>

#include <string>
#include <vector>

#pragma once

namespace pangolin
{

struct TableLoaderInterface {
  virtual bool ReadRow(std::vector<std::string>& row) = 0;
};

}  // namespace pangolin
