#pragma once

#include "base/memory/ref_counted.h"
#include "azer/math/math.h"
#include "azer/render/render.h"
#include "lordaeron/effect/light.h"

namespace lord {
namespace sandbox {
class MyEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  MyEffect(azer::VertexDescPtr desc);
  ~MyEffect();

  const char* GetEffectName() const override;
  bool Init(const ShaderPrograms& source) override;

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
    azer::Vector4 color;
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
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitTechnique(const ShaderPrograms& source);
  void InitGpuConstantTable();

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::Vector4 camerapos_;
  azer::TexturePtr diffuse_map_;
  float ambient_scalar_;
  float specular_scalar_;
  DirLight dir_light_;
  PointLight point_light_;
  SpotLight spot_light_;
  DISALLOW_COPY_AND_ASSIGN(MyEffect);
};

class MaterialProvider : public azer::EffectParamsProvider {
 public:
  static const char kEffectParamsProviderName[];
  MaterialProvider();
  const char* name() const override;
  void UpdateParams(const FrameArgs& args) override {}
  void InitFromConfigNode(azer::ConfigNode* config, azer::FileSystem* fs);
 private:
  float ambient_;
  float specular_;
  azer::Vector4 emission_;
  azer::TexturePtr diffuse_map_;
  azer::TexturePtr nmh_map_;
  DISALLOW_COPY_AND_ASSIGN(MaterialProvider);
};

typedef scoped_refptr<MyEffect> MyEffectPtr;
MyEffectPtr CreateMyEffect();
}  // namespace sandbox
}  // namespace lord
