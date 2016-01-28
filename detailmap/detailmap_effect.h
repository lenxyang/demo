#pragma once

#include "azer/render/render.h"

class DetailmapEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  DetailmapEffect();
  ~DetailmapEffect();

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(azer::VertexDesc* desc, const ShaderPrograms& sources) override;
#pragma pack(push, 4)
  struct ds_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
    azer::Vector4 eyepos;
  };

  struct hs_cbuffer {
    azer::Vector4 edge;
  };
#pragma pack(pop)
  void SetPV(const azer::Matrix4& value) { pv_ = value;}
  void SetWorld(const azer::Matrix4& value) { world_ = value;}
  void SetEyePos(const azer::Vector4& pos) { eyepos_ = pos;}
  void SetEdgeInside(const azer::Vector4& value) {edge_ = value;}
  void SetDiffuseMap(azer::Texture* ptr) { diffusemap_ = ptr;}
  void SetNMMap(azer::Texture* ptr) { nmmap_ = ptr;}
 protected:
  void UseTexture(azer::Renderer* renderer) override;
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitGpuConstantTable();
  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::Vector4 eyepos_;
  azer::Vector4 edge_;
  azer::TexturePtr diffusemap_;
  azer::TexturePtr nmmap_;
  DISALLOW_COPY_AND_ASSIGN(DetailmapEffect);
};

typedef scoped_refptr<DetailmapEffect> DetailmapEffectPtr;


DetailmapEffectPtr CreateDetailmapEffect();
