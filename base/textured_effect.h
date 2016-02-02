#pragma once

#include "azer/render/effect_creator.h"
#include "azer/render/render.h"
#include "lordaeron/effect/material.h"
#include "lordaeron/effect/light.h"
#include "lordaeron/scene/render_node.h"

class TexturedEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  TexturedEffect();
  ~TexturedEffect();

  const char* GetEffectName() const override;
  bool Init(azer::VertexDesc* desc, const azer::Shaders& source) override;

#pragma pack(push, 4)
  struct vs_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
    azer::Vector4 camerapos;
  };

  struct ps_cbuffer {
    lord::DirLight light;
    lord::PointLight pointlight;
    lord::SpotLight  spotlight;
    float ambient_scalar;
    float specular_scalar;
    float alpha;
    float pad2;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value);
  void SetWorld(const azer::Matrix4& value);
  void SetCameraPos(const azer::Vector4& CameraPos);
  void SetDirLight(const lord::DirLight& value);
  void SetPointLight(const lord::PointLight& value);
  void SetSpotLight(const lord::SpotLight& value);
  void set_ambient_scalar(float scalar) { ambient_scalar_ = scalar;}
  void set_specular_scalar(float scalar) { specular_scalar_ = scalar;}
  void set_diffuse_texture(azer::Texture* tex) { diffuse_map_ = tex;}
  void set_alpha(float alpha) { alpha_ = alpha;}

  static azer::Effect* CreateObject() { return new TexturedEffect;}
 protected:
  void UseTexture(azer::Renderer* renderer) override;
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitTechnique(const azer::Shaders& source);
  void InitGpuConstantTable();

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::Vector4 camerapos_;
  azer::TexturePtr diffuse_map_;
  float ambient_scalar_;
  float specular_scalar_;
  float alpha_;
  lord::DirLight dir_light_;
  lord::PointLight point_light_;
  lord::SpotLight spot_light_;
  DECLARE_EFFECT_DYNCREATE(TexturedEffect);
  DISALLOW_COPY_AND_ASSIGN(TexturedEffect);
};

using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;

class RenderNodeTexEffectAdapter : public EffectParamsAdapter {
 public:
  RenderNodeTexEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(RenderNodeTexEffectAdapter);
};

class LordEnvNodeDelegateTexEffectAdapter : public EffectParamsAdapter {
 public:
  LordEnvNodeDelegateTexEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(LordEnvNodeDelegateTexEffectAdapter);
};

class SdkMeshMaterialTexEffectAdapter : public EffectParamsAdapter {
 public:
  SdkMeshMaterialTexEffectAdapter();
  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(SdkMeshMaterialTexEffectAdapter);
};

class CameraProviderTexEffectAdapter : public EffectParamsAdapter {
 public:
  CameraProviderTexEffectAdapter();
  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(CameraProviderTexEffectAdapter);
};

class WorldProviderTexEffectAdapter : public EffectParamsAdapter {
 public:
  WorldProviderTexEffectAdapter();
  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(WorldProviderTexEffectAdapter);
};

class LightProviderTexEffectAdapter : public EffectParamsAdapter {
 public:
  LightProviderTexEffectAdapter();
  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(LightProviderTexEffectAdapter);
};
