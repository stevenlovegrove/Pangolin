#include <pangolin/windowing/window.h>
#include <pangolin/factory/factory_registry.h>

namespace pangolin
{

std::unique_ptr<WindowInterface> CreateWindow(const Uri& uri)
{
    return FactoryRegistry::I()->Construct<WindowInterface>(uri);
}

}
