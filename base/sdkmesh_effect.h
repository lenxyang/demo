#pragma once

#include <string>
#include <vector>

#include "azer/render/render.h"

class SdkMeshMaterial : public azer::EffectParamsProvider {
 public:
  SdkMeshMaterial();
  const char* GetProviderName() const override;

  azer::Texture* diffusemap() const { return diffusemap_.get();}
  azer::Texture* normalmap() const { return normalmap_.get();}
  azer::Texture* specularmap() const { return specularmap_.get();}
  const azer::Vector4& ambient() const { return ambient_;}
  const azer::Vector4& diffuse() const { return diffuse_;}
  const azer::Vector4& emissive() const { return emissive_;}
  const azer::Vector4& specular() const { return specular_;}
 private:
  azer::TexturePtr diffusemap_;
  azer::TexturePtr normalmap_;
  azer::TexturePtr specularmap_;
  azer::Vector4 ambient_;
  azer::Vector4 diffuse_;
  azer::Vector4 emissive_;
  azer::Vector4 specular_;
  float specular_power_;
  float alpha_;
  
  std::string name_;
  DISALLOW_COPY_AND_ASSIGN(SdkMeshMaterial);
};

class SdkMeshEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  SdkMeshEffect();
  ~SdkMeshEffect() {}

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(azer::VertexDesc* desc, const ShaderPrograms& sources) override;

#pragma pack(push, 4)
  struct vs_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
    azer::Vector4 camerapos;
  };
  struct ps_cbuffer {
    lord::DirLight   dirlight;
    lord::SpotLight  spotlight;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value) { pv_ = value;}
  void SetWorld(const azer::Matrix4& value) { world_ = value;}
  void SetDiffuseMap(azer::Texture* ptr) { diffusemap_ = ptr;}
  void SetNormalMap(azer::Texture* ptr) { normalmap_ = ptr;}
  void SetSpecularMap(azer::Texture* ptr) { specularmap_ = ptr;}
  void SetPointLight(const lord::PointLight& value);
  void SetSpotLight(const lord::SpotLight& value);
 protected:
  void UseTexture(azer::Renderer* renderer) override;
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitGpuConstantTable();
  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::Vector4 camerapos_;
  lord::DirLight dir_light_;
  lord::SpotLight spot_light_;
  azer::TexturePtr diffusemap_;
  azer::TexturePtr normalmap_;
  azer::TexturePtr specularmap_;
  DISALLOW_COPY_AND_ASSIGN(SdkMeshEffect);
};

scoped_refptr<SdkMeshEffect> CreateSdkMeshEffect();

using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;

class SdkMeshMaterialEffectAdapter : public EffectParamsAdapter {
 public:
  SdkMeshMaterialEffectAdapter();
  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(SdkMeshMaterialEffectAdapter);
};

class CameraProviderSdkMeshAdapter : public EffectParamsAdapter {
 public:
  CameraProviderSdkMeshAdapter();
  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(CameraProviderSdkMeshAdapter);
};
