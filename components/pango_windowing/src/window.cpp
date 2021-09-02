#include <pangolin/windowing/window.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/factory/RegisterFactoriesWindowInterface.h>

namespace pangolin
{

std::unique_ptr<WindowInterface> ConstructWindow(const Uri& uri)
{
    RegisterFactoriesWindowInterface();
    return FactoryRegistry::I()->Construct<WindowInterface>(uri);
}

}
