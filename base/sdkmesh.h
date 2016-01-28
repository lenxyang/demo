#pragma once

#include <string>
#include <vector>

#include "azer/base/file_system.h"
#include "azer/render/render.h"
#include "lordaeron/resource/resource_loader.h"

class SdkMeshMaterial;
typedef scoped_refptr<SdkMeshMaterial> SdkMeshMaterialPtr;

class SdkMeshData {
 public:
  SdkMeshData(azer::FileSystem* fs);

  struct Material {
    std::string         name;
    bool                per_vertex_color;
    float               specular_power;
    float               alpha;
    azer::Vector3       ambient_color;
    azer::Vector3       diffuse_color;
    azer::Vector3       specular_color;
    azer::Vector3       emissive_color;
    std::string         diffuse_texture;
    std::string         normal_texture;
    std::string         specular_texture;
  };
  
  struct Subset {
    int32 vertex_base;
    int32 start_index;
    int32 vertex_data_index;
    int32 indices_data_index;
    int32 material_index;
    azer::PrimitiveTopology primitive;
  };

  struct Mesh {
    std::string name;
    bool ccw;
    azer::Vector3 center;
    azer::Vector3 extends;
    std::vector<Subset> subsets;
  };

  bool LoadFromFile(const azer::ResPath& path);
  bool CreateMesh(std::vector<azer::MeshPtr>* meshes, 
                  azer::EffectAdapterContext* ctx);
  SdkMeshMaterialPtr CreateMaterial(int32 index);
  const Subset& GetSubset(int32 mesh_index, int32 part_index);
  azer::EntityPtr CreateEntity(int32 mesh_index, int32 part_index);

  int32 mesh_count() const { return static_cast<int32>(meshes_.size());}
  const Mesh& mesh_at(int32 index) { return meshes_[index];}
  int32 material_count() const { return static_cast<int32>(mtrls_.size());}
  const Material& mtrl_at(int32 index) { return mtrls_[index];}
 private:
  bool LoadFromData(const uint8* data, int32 size);
  bool LoadVertexData(const uint8* data, int32 size);
  bool LoadIndicesData(const uint8* data, int32 size);
  bool LoadMaterial(const uint8* data, int32 size);
  bool LoadMesh(const uint8* data, int32 size);

  std::vector<Material> mtrls_;
  std::vector<Mesh> meshes_;
  std::vector<azer::SlotVertexDataPtr> vdata_vec_;
  std::vector<azer::IndicesDataPtr> idata_vec_;
  std::vector<azer::VertexBufferPtr> vbs_;
  std::vector<azer::IndicesBufferPtr> ibs_;
  azer::ResPath model_path_;
  azer::FileSystem* filesystem_;
  DISALLOW_COPY_AND_ASSIGN(SdkMeshData);
};

class SdkMeshSpecialLoader : public lord::ResourceSpecialLoader {
 public:
  SdkMeshSpecialLoader();
  const char* GetLoaderName() const override;
  bool CouldLoad(azer::ConfigNode* node) const override;
  lord::VariantResource Load(const azer::ConfigNode* node,
                             lord::ResourceLoadContext* ctx) override;
 private:
  DISALLOW_COPY_AND_ASSIGN(SdkMeshSpecialLoader);
};
