#include "demo/displacement_mapping/effect_adapter.h"

#include "azer/render/util.h"
#include "demo/displacement_mapping/effect.h"
#include "lordaeron/effect/diffuse_effect.h"
#include "lordaeron/scene/scene_node.h"
#include "lordaeron/scene/scene_render_tree.h"
#include "demo/displacement_mapping/effect.h"

namespace lord {
namespace sandbox {
using namespace azer;
MaterialEffectAdapter::MaterialEffectAdapter() {
}

EffectAdapterKey MaterialEffectAdapter::key() const {
  return std::make_pair(typeid(MyEffect).name(),
                        typeid(MaterialProvider).name());
}

void MaterialEffectAdapter::Apply(Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MyEffect));
  CHECK(typeid(*params) == typeid(MaterialProvider));
  MaterialProvider* provider = (MaterialProvider*)params;
  MyEffect* effect = dynamic_cast<MyEffect*>(e);
  effect->set_ambient_scalar(provider->ambient_scalar());
  effect->set_specular_scalar(provider->specular_scalar());
  effect->set_diffuse_texture(provider->diffuse_map());
  effect->set_nmh_texture(provider->nmh_map());
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
