#include <pangolin/factory/RegisterFactoriesWindowInterface.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/windowing/window.h>

namespace pangolin
{

// HACK: global variable to share ref to WindowInterface ourside of the URI
// factory mechanism.
std::shared_ptr<WindowInterface> context_to_share = nullptr;

Shared<WindowInterface> WindowInterface::Create(
    const WindowInterface::Params& params)
{
  RegisterFactoriesWindowInterface();
  context_to_share = params.shared_context;
  auto win = FactoryRegistry::I()->Construct<WindowInterface>(params.uri);
  context_to_share = nullptr;
  return win;
}

}  // namespace pangolin
