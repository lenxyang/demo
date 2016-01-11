#pragma once

#include "azer/render/effect_creator.h"
#include "azer/render/render.h"
#include "lordaeron/effect/material.h"
#include "lordaeron/effect/light.h"
#include "lordaeron/scene/render_node.h"

class TexMaterial : public lord::Material {
 public:
  static const char kEffectProviderName[];
  TexMaterial();
  const char* GetProviderName() const override;
  bool Init(const azer::ConfigNode* node, lord::ResourceLoadContext* ctx) override;
  float ambient_scalar() const { return ambient_scalar_;}
  float specular_scalar() const { return specular_scalar_;}
  const azer::Vector4& emission() const { return emission_;}
  azer::Texture* diffuse_map() { return diffuse_map_.get();}
  static azer::EffectParamsProvider* CreateObject() { return new TexMaterial;}
 private:
  float ambient_scalar_;
  float specular_scalar_;
  azer::Vector4 emission_;
  azer::TexturePtr diffuse_map_;
  azer::TexturePtr nmh_map_;
  DECLARE_EFFECT_PROVIDER_DYNCREATE(TexMaterial);
  DISALLOW_COPY_AND_ASSIGN(TexMaterial);
};


using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;

class TexMaterialEffectAdapter : public EffectParamsAdapter {
 public:
  TexMaterialEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(TexMaterialEffectAdapter);
};


class ShadowMapMaterialEffectAdapter : public EffectParamsAdapter {
 public:
  ShadowMapMaterialEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(ShadowMapMaterialEffectAdapter);
};

