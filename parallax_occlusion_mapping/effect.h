#pragma once

#include "base/memory/ref_counted.h"
#include "azer/math/math.h"
#include "azer/base/config_node.h"
#include "azer/base/file_system.h"
#include "azer/render/render.h"
#include "azer/render/effect_creator.h"
#include "lordaeron/effect/light.h"
#include "lordaeron/effect/material.h"

namespace lord {
namespace sandbox {
class MyEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  MyEffect();
  ~MyEffect();

  const char* GetEffectName() const override;
  bool Init(azer::VertexDesc* vertex, const ShaderPrograms& source) override;

#pragma pack(push, 4)
  struct vs_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
    azer::Vector4 camerapos;
  };

  struct ps_cbuffer {
    DirLight light;
    PointLight pointlight;
    SpotLight  spotlight;
    float ambient_scalar;
    float specular_scalar;
    float pad1;
    float pad2;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value);
  void SetWorld(const azer::Matrix4& value);
  void SetCameraPos(const azer::Vector4& CameraPos);
  void SetDirLight(const DirLight& value);
  void SetPointLight(const PointLight& value);
  void SetSpotLight(const SpotLight& value);
  void set_ambient_scalar(float scalar) { ambient_scalar_ = scalar;}
  void set_specular_scalar(float scalar) { specular_scalar_ = scalar;}
  void set_diffuse_texture(azer::Texture* tex) { diffuse_map_ = tex;}
  void set_nmh_texture(azer::Texture* tex) { nmh_map_ = tex;}

  static azer::Effect* CreateObject() { return new MyEffect;}
 protected:
  void UseTexture(azer::Renderer* renderer) override;
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitTechnique(const ShaderPrograms& source);
  void InitGpuConstantTable();

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::Vector4 camerapos_;
  azer::TexturePtr diffuse_map_;
  azer::TexturePtr nmh_map_;
  float ambient_scalar_;
  float specular_scalar_;
  DirLight dir_light_;
  PointLight point_light_;
  SpotLight spot_light_;
  DECLARE_EFFECT_DYNCREATE(MyEffect);
  DISALLOW_COPY_AND_ASSIGN(MyEffect);
};

class MaterialProvider : public Material {
 public:
  static const char kEffectProviderName[];
  MaterialProvider();
  const char* GetProviderName() const override;
  bool Init(const azer::ConfigNode* node, ResourceLoadContext* ctx) override;
  float ambient_scalar() const { return ambient_scalar_;}
  float specular_scalar() const { return specular_scalar_;}
  const azer::Vector4& emission() const { return emission_;}
  azer::Texture* diffuse_map() { return diffuse_map_.get();}
  azer::Texture* nmh_map() { return nmh_map_.get();};
  static azer::EffectParamsProvider* CreateObject() { return new MaterialProvider;}
 private:
  float ambient_scalar_;
  float specular_scalar_;
  azer::Vector4 emission_;
  azer::TexturePtr diffuse_map_;
  azer::TexturePtr nmh_map_;
  DECLARE_EFFECT_PROVIDER_DYNCREATE(MaterialProvider);
  DISALLOW_COPY_AND_ASSIGN(MaterialProvider);
};

typedef scoped_refptr<MyEffect> MyEffectPtr;

using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;

class MaterialEffectAdapter : public EffectParamsAdapter {
 public:
  MaterialEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(MaterialEffectAdapter);
};

class RenderNodeEffectAdapter : public EffectParamsAdapter {
 public:
  RenderNodeEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(RenderNodeEffectAdapter);
};

class LordEnvNodeDelegateEffectAdapter : public EffectParamsAdapter {
 public:
  LordEnvNodeDelegateEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(LordEnvNodeDelegateEffectAdapter);
};
}  // namespace sandbox
}  // namespace lord
