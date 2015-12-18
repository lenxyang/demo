#include "demo/parallax_occlusion_mapping/effect_adapter.h"

#include "azer/render/util.h"
#include "lordaeron/sandbox/lighting/effect.h"
#include "lordaeron/effect/diffuse_effect.h"
#include "lordaeron/scene/scene_node.h"
#include "lordaeron/scene/scene_render_tree.h"

namespace lord {
namespace sandbox {
using namespace azer;
ColorEffectAdapter::ColorEffectAdapter() {
}

EffectAdapterKey ColorEffectAdapter::key() const {
  return std::make_pair(typeid(MyEffect).name(),
                        typeid(DiffuseColorProvider).name());
}

void ColorEffectAdapter::Apply(Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MyEffect));
  CHECK(typeid(*params) == typeid(DiffuseColorProvider));
  DiffuseColorProvider* provider = (DiffuseColorProvider*)params;
  MyEffect* effect = dynamic_cast<MyEffect*>(e);
  effect->SetColor(provider->color());
}

// class SceneRenderNodeMyEffectAdapter
SceneRenderNodeEffectAdapter::SceneRenderNodeEffectAdapter() {}
EffectAdapterKey SceneRenderNodeEffectAdapter::key() const {
  return std::make_pair(typeid(MyEffect).name(),
                        typeid(SceneRenderNode).name());
}

void SceneRenderNodeEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MyEffect));
  CHECK(typeid(*params) == typeid(SceneRenderNode));
  const SceneRenderNode* provider = (const SceneRenderNode*)params;
  MyEffect* effect = dynamic_cast<MyEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->camera()->GetProjViewMatrix());
  effect->SetCameraPos(Vector4(provider->camera()->position(), 1.0f));
}

SceneRenderEnvNodeEffectAdapter::SceneRenderEnvNodeEffectAdapter() {
}

EffectAdapterKey SceneRenderEnvNodeEffectAdapter::key() const {
  return std::make_pair(typeid(MyEffect).name(),
                        typeid(SceneRenderEnvNode).name());
}

void SceneRenderEnvNodeEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MyEffect));
  CHECK(typeid(*params) == typeid(SceneRenderEnvNode));
  const SceneRenderEnvNode* provider = (const SceneRenderEnvNode*)params;
  MyEffect* effect = dynamic_cast<MyEffect*>(e);
  for (auto iter = provider->lights().begin(); 
       iter != provider->lights().end();
       ++iter) {
    Light* light = iter->get();
    if (light->type() == kDirectionalLight) {
      effect->SetDirLight(light->dir_light());
    } else if (light->type() == kPointLight) {
      effect->SetPointLight(light->point_light());
    } else if (light->type() == kSpotLight) {
      effect->SetSpotLight(light->spot_light());
    }
  }
}
}  // namespace sandbox
}  // namespace lord
