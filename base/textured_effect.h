#pragma once

#include "azer/render/effect_creator.h"
#include "azer/render/render.h"
#include "lordaeron/effect/material.h"
#include "lordaeron/effect/light.h"
#include "lordaeron/scene/scene_render_tree.h"

class TexturedEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  TexturedEffect();
  ~TexturedEffect();

  const char* GetEffectName() const override;
  bool Init(azer::VertexDesc* desc, const ShaderPrograms& source) override;

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
    float pad1;
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

  static azer::Effect* CreateObject() { return new TexturedEffect;}
 protected:
  void UseTexture(azer::Renderer* renderer) override;
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitTechnique(const ShaderPrograms& source);
  void InitGpuConstantTable();

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::Vector4 camerapos_;
  azer::TexturePtr diffuse_map_;
  float ambient_scalar_;
  float specular_scalar_;
  lord::DirLight dir_light_;
  lord::PointLight point_light_;
  lord::SpotLight spot_light_;
  DECLARE_EFFECT_DYNCREATE(TexturedEffect);
  DISALLOW_COPY_AND_ASSIGN(TexturedEffect);
};


class TexMaterial : public lord::Material {
 public:
  static const char kEffectProviderName[];
  TexMaterial();
  const char* GetProviderName() const override;
  bool Init(const azer::ConfigNode* node, lord::ResourceLoadContext* ctx) override;
  void UpdateParams(const azer::FrameArgs& args) override {}
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

class SceneRenderNodeTexEffectAdapter : public EffectParamsAdapter {
 public:
  SceneRenderNodeTexEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(SceneRenderNodeTexEffectAdapter);
};

class SceneRenderEnvNodeTexEffectAdapter : public EffectParamsAdapter {
 public:
  SceneRenderEnvNodeTexEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(SceneRenderEnvNodeTexEffectAdapter);
};
