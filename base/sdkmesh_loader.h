#pragma once

#include "base/files/file_path.h"
#include "azer/base/res_path.h"
#include "azer/base/file_system.h"
#include "azer/render/render.h"
#include "lordaeron/effect/material.h"

struct SDKMESH_MATERIAL;
struct SdkMesh {
  std::string name;
  std::vector<azer::MeshPartPtr> part;
  bool ccw;
  bool pmalpha;
  azer::Vector3 center;
  azer::Vector3 extents;
};

struct SdkModel {
  std::vector<SdkMesh> meshes;
  std::vector<scoped_refptr<SdkMeshMaterial> > materials;
};


class SdkMeshMaterial : public azer::EffectParamsProvider {
 public:
  static const char kEffectProviderName[];
  SdkMeshMaterial();

  const char* GetProviderName() const override;

  azer::Texture* texture1() const { return texture1_.get();}
  azer::Texture* texture2() const { return texture2_.get();}
  const azer::Vector4& ambient() const { return ambient_;}
  const azer::Vector4& diffuse() const { return diffuse_;}
  const azer::Vector4& emissive() const { return emissive_;}
  const azer::Vector4& specular() const { return specular_;}
  float alpha() { return alpha_;}
  const std::string& name() const { return name_;}
 private:
  azer::TexturePtr texture1_;
  azer::TexturePtr texture2_;
  azer::Vector4 ambient_;
  azer::Vector4 diffuse_;
  azer::Vector4 emissive_;
  azer::Vector4 specular_;
  float alpha_;
  float specular_power_;
  std::string name_;

  friend void InitFromSDKMaterial(const SDKMESH_MATERIAL& mh,
                                  bool enable_vertex_color,
                                  bool enable_skinned,
                                  bool enable_dual_texture,
                                  azer::FileSystem* fs,
                                  SdkMeshMaterial* mtrl);
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
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value) { pv_ = value;}
  void SetWorld(const azer::Matrix4& value) { world_ = value;}
  void SetDiffuseMap(Texture* ptr) { diffusemap_ = ptr;}
  void SetSpecularMap(Texture* ptr) { specular_ = ptr;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitGpuConstantTable();
  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  azer::TexturePtr diffusemap_;
  azer::TexturePtr specularmap_;
  DISALLOW_COPY_AND_ASSIGN(SdkMeshEffect);
};


bool LoadSDKModel(const ::base::FilePath& path, SdkModel* model);
bool LoadSDKModel(const azer::ResPath& path, azer::FileSystem* fs, SdkModel* model);
