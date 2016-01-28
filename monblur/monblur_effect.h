#pragma once

#include <string>
#include <vector>

#include "azer/render/render.h"
#include "demo/base/base.h"
#include "demo/base/sdkmesh_effect.h"

class MonblurEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  MonblurEffect();
  ~MonblurEffect() {}

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(azer::VertexDesc* desc, const ShaderPrograms& sources) override;

  static const int32 kMaxStep = 3;
#pragma pack(push, 4)
  struct gs_cbuffer {
    azer::Matrix4 pv[kMaxStep];
    azer::Matrix4 world[kMaxStep];
    azer::Vector4 camerapos;
  };
  struct ps_cbuffer {
    lord::DirLight   dirlight;
    lord::SpotLight  spotlight;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value);
  void SetWorld(const azer::Matrix4& value);
  void SetCameraPos(const azer::Vector4& pos);
  void SetDiffuseMap(azer::Texture* ptr) { diffusemap_ = ptr;}
  void SetNormalMap(azer::Texture* ptr) { normalmap_ = ptr;}
  void SetSpecularMap(azer::Texture* ptr) { specularmap_ = ptr;}
  void SetDirLight(const lord::DirLight& value);
  void SetSpotLight(const lord::SpotLight& value);
  static azer::Effect* CreateObject() { return new MonblurEffect;}
 protected:
  void UseTexture(azer::Renderer* renderer) override;
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitGpuConstantTable();
  azer::Matrix4 pv_[kMaxStep];
  azer::Matrix4 world_[kMaxStep];
  azer::Vector4 camerapos_;
  lord::DirLight dir_light_;
  lord::SpotLight spot_light_;
  azer::TexturePtr diffusemap_;
  azer::TexturePtr normalmap_;
  azer::TexturePtr specularmap_;
  DECLARE_EFFECT_DYNCREATE(MonblurEffect);
  DISALLOW_COPY_AND_ASSIGN(MonblurEffect);
};

scoped_refptr<MonblurEffect> CreateMonblurEffect();

using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;

class MonblurMaterialEffectAdapter : public EffectParamsAdapter {
 public:
  MonblurMaterialEffectAdapter();
  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(MonblurMaterialEffectAdapter);
};

class RenderNodeMonblurEffectAdapter : public EffectParamsAdapter {
 public:
  RenderNodeMonblurEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(RenderNodeMonblurEffectAdapter);
};

class LordEnvNodeDelegateMonblurEffectAdapter : public EffectParamsAdapter {
 public:
  LordEnvNodeDelegateMonblurEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(LordEnvNodeDelegateMonblurEffectAdapter);
};
