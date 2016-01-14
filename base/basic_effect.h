#pragma once

#include "base/memory/ref_counted.h"
#include "azer/math/math.h"
#include "azer/render/render.h"
#include "azer/render/effect_creator.h"
#include "lordaeron/effect/light.h"
#include "lordaeron/effect/material.h"

class BasicEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  BasicEffect();
  ~BasicEffect();

  const char* GetEffectName() const override;
  bool Init(azer::VertexDesc* desc, const ShaderPrograms& source) override;

#pragma pack(push, 4)
  struct vs_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
  };

  struct ps_cbuffer {
    azer::Vector4 color;
    azer::Vector4 emission;
    lord::DirLight light;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value);
  void SetWorld(const azer::Matrix4& value);
  void SetColor(const azer::Vector4& value);
  void SetEmission(const azer::Vector4& value);
  void SetDirLight(const lord::DirLight& value);
  static azer::Effect* CreateObject() { return new BasicEffect;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitTechnique(const ShaderPrograms& source);
  void InitGpuConstantTable();

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::Vector4 color_;
  azer::Vector4 emission_;
  lord::DirLight light_;
  DECLARE_EFFECT_DYNCREATE(BasicEffect);
  DISALLOW_COPY_AND_ASSIGN(BasicEffect);
};

class BasicColorProvider : public lord::Material {
 public:
  static const char kEffectProviderName[];
  BasicColorProvider();
  ~BasicColorProvider();

  const char* GetProviderName() const override;
  bool Init(const azer::ConfigNode* node, lord::ResourceLoadContext* ctx) override;
  azer::Vector4 color() const { return color_;}
  void SetColor(const azer::Vector4& color) { color_ = color;}
  static azer::EffectParamsProvider* CreateObject() { return new BasicColorProvider;}
 private:
  azer::Vector4 color_;
  DECLARE_EFFECT_PROVIDER_DYNCREATE(BasicColorProvider);
  DISALLOW_COPY_AND_ASSIGN(BasicColorProvider);
};

using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;

class BasicColorEffectAdapter : public EffectParamsAdapter {
 public:
  BasicColorEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(BasicColorEffectAdapter);
};

class RenderNodeBasicEffectAdapter : public EffectParamsAdapter {
 public:
  RenderNodeBasicEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(RenderNodeBasicEffectAdapter);
};

class LordEnvNodeBasicEffectAdapter : public EffectParamsAdapter {
 public:
  LordEnvNodeBasicEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(LordEnvNodeBasicEffectAdapter);
};
