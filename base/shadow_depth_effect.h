#pragma once

#include "azer/render/render.h"

class ShadowEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  ShadowEffect();
  ~ShadowEffect();

  const char* GetEffectName() const override;
  bool Init(const ShaderPrograms& source) override;

#pragma pack(push, 4)
  struct vs_cbuffer {
    azer::Matrix4 pvw;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value);
  void SetWorld(const azer::Matrix4& value);
  static azer::Effect* CreateObject() { return new ShadowEffect;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitTechnique(const ShaderPrograms& source);
  void InitGpuConstantTable();

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  DECLARE_EFFECT_DYNCREATE(ShadowEffect);
  DISALLOW_COPY_AND_ASSIGN(ShadowEffect);
};
