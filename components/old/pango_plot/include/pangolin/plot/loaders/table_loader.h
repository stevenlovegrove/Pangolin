#include <pangolin/platform.h>
#include <vector>
#include <string>

namespace pangolin {

struct TableLoaderInterface
{
    virtual bool ReadRow(std::vector<std::string>& row) = 0;
};

}
