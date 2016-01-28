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
  };

  struct hs_cbuffer {
    azer::Vector4 edge;
    azer::Vector4 inside;
  };

  struct ps_cbuffer {
    azer::Vector4 color;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value) { pv_ = value;}
  void SetWorld(const azer::Matrix4& value) { world_ = value;}
  void SetColor(const azer::Vector4& value) {color_ = value;}
  void SetEdge(const azer::Vector4& value) {edge_ = value;}
  void SetInside(const azer::Vector4& value) {inside_ = value;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitGpuConstantTable();
  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::Vector4 color_;
  azer::Vector4 edge_;
  azer::Vector4 inside_;
  DISALLOW_COPY_AND_ASSIGN(DetailmapEffect);
};

typedef scoped_refptr<DetailmapEffect> DetailmapEffectPtr;


DetailmapEffectPtr CreateDetailmapEffect();
