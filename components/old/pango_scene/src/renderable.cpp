#include <pangolin/scene/renderable.h>

namespace pangolin
{

Renderable::guid_t Renderable::UniqueGuid()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return (guid_t)gen();
}

Renderable::Renderable(std::weak_ptr<Renderable> const& parent) :
    guid(UniqueGuid()),
    parent(parent),
    T_pc(IdentityMatrix()),
    should_show(true)
{
}

Renderable::~Renderable() {}

// Default implementation simply renders children.
void Renderable::Render(RenderParams const& params) { RenderChildren(params); }

void Renderable::RenderChildren(RenderParams const& params)
{
  for (auto& p : children) {
    Renderable& r = *p.second;
    if (r.should_show) {
      glPushMatrix();
      r.T_pc.Multiply();
      r.Render(params);
      if (r.manipulator) {
        r.manipulator->Render(params);
      }
      glPopMatrix();
    }
  }
}

std::shared_ptr<Renderable> Renderable::FindChild(guid_t guid)
{
  auto o = children.find(guid);
  if (o != children.end()) {
    return o->second;
  }

  for (auto& kv : children) {
    std::shared_ptr<Renderable> c = kv.second->FindChild(guid);
    if (c) return c;
  }

  return std::shared_ptr<Renderable>();
}

Renderable& Renderable::Add(std::shared_ptr<Renderable> const& child)
{
  if (child) {
    children[child->guid] = child;
  };
  return *this;
}

}  // namespace pangolin
