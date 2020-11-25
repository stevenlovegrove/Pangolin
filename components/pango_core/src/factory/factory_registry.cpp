#include "pangolin/factory/factory_registry.h"

namespace pangolin {

std::shared_ptr<FactoryRegistry> FactoryRegistry::I()
{
    static std::shared_ptr<FactoryRegistry> registry(new FactoryRegistry());
    return registry;
}

}
