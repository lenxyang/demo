#pragma once

#include "azer/render/render.h"

class ParticleEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  ParticleEffect();
  ~ParticleEffect();

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(azer::VertexDesc* desc, const azer::Shaders& sources) override;

#pragma pack(push, 4)
  struct vs_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
  };

  struct ps_cbuffer {
    azer::Vector4 color;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value) { pv_ = value;}
  void SetWorld(const azer::Matrix4& value) { world_ = value;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitGpuConstantTable();

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::TexturePtr texture_;
  azer::TexturePtr random_;
  DISALLOW_COPY_AND_ASSIGN(ParticleEffect);
};

class Particle {
 public:
  Particle();
  void Render(azer::Renderer* renderer);
  void SetVertexBuffer(uint32 index, azer::VertexBuffer* vb);
 private:
  azer::VertexBufferPtr vbs_[2];
  int32 which_;
  BlendingPtr blending_;
  DISALLOW_COPY_AND_ASSIGN(Particle);
};

