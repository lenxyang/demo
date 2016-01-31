#include "demo/base/material.h"

#include "base/strings/utf_string_conversions.h"
#include "lordaeron/env.h"
#include "lordaeron/resource/resource_util.h"
#include "lordaeron/resource/resource_loader.h"
#include "demo/base/resource_util.h"
#include "demo/base/textured_effect.h"
#include "demo/base/shadowmap_effect.h"

using namespace lord;
using namespace azer;

using base::UTF8ToUTF16;

// class TexMaterial
IMPLEMENT_EFFECT_PROVIDER_DYNCREATE(TexMaterial);
const char TexMaterial::kEffectProviderName[] = "TexMaterial";
TexMaterial::TexMaterial() {
}

bool TexMaterial::Init(const azer::ConfigNode* node, ResourceLoadContext* ctx) {
  CHECK(node->GetChildTextAsFloat("ambient", &ambient_scalar_));
  CHECK(node->GetChildTextAsFloat("specular", &specular_scalar_));
  CHECK(node->GetChildTextAsVec4("emission", &emission_));
  std::string diffusemap_path;
  CHECK(node->GetChildText("diffusemap", &diffusemap_path));
  diffuse_map_ = Load2DTexture(ResPath(UTF8ToUTF16(diffusemap_path)), ctx->filesystem);
  return true;
}

const char* TexMaterial::GetProviderName() const { return kEffectParamsProviderName;}


// class TexMaterialEffectAdapter
TexMaterialEffectAdapter::TexMaterialEffectAdapter() {}
EffectAdapterKey TexMaterialEffectAdapter::key() const {
  return std::make_pair(typeid(TexturedEffect).name(),
                        typeid(TexMaterial).name());
}
void TexMaterialEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(TexturedEffect));
  CHECK(typeid(*params) == typeid(TexMaterial));
  TexMaterial* provider = (TexMaterial*)params;
  TexturedEffect* effect = dynamic_cast<TexturedEffect*>(e);
  effect->set_ambient_scalar(provider->ambient_scalar());
  effect->set_specular_scalar(provider->specular_scalar());
  effect->set_diffuse_texture(provider->diffuse_map());
}



// class ShadowMapMaterialEffectAdapter
ShadowMapMaterialEffectAdapter::ShadowMapMaterialEffectAdapter() {}
EffectAdapterKey ShadowMapMaterialEffectAdapter::key() const {
  return std::make_pair(typeid(ShadowMapEffect).name(),
                        typeid(TexMaterial).name());
}
void ShadowMapMaterialEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(ShadowMapEffect));
  CHECK(typeid(*params) == typeid(TexMaterial));
  TexMaterial* provider = (TexMaterial*)params;
  ShadowMapEffect* effect = dynamic_cast<ShadowMapEffect*>(e);
  effect->set_ambient_scalar(provider->ambient_scalar());
  effect->set_specular_scalar(provider->specular_scalar());
  effect->set_diffuse_texture(provider->diffuse_map());
}

