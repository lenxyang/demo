#pragma once

#include "azer/render/effect_creator.h"
#include "azer/render/render.h"
#include "lordaeron/scene/scene_render_tree.h"
#include "lordaeron/resource/resource_util.h"

class ShadowDepthEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  ShadowDepthEffect();
  ~ShadowDepthEffect();

  const char* GetEffectName() const override;
  bool Init(const ShaderPrograms& source) override;

#pragma pack(push, 4)
  struct vs_cbuffer {
    azer::Matrix4 pvw;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value);
  void SetWorld(const azer::Matrix4& value);
  static azer::Effect* CreateObject() { return new ShadowDepthEffect();}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitTechnique(const ShaderPrograms& source);
  void InitGpuConstantTable();

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  DECLARE_EFFECT_DYNCREATE(ShadowDepthEffect);
  DISALLOW_COPY_AND_ASSIGN(ShadowDepthEffect);
};


using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;
class SceneRenderNodeDepthEffectAdapter : public EffectParamsAdapter {
 public:
  SceneRenderNodeDepthEffectAdapter() {}

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(SceneRenderNodeDepthEffectAdapter);
};
