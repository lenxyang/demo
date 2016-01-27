#pragma once

#include <string>
#include <vector>

#include "azer/base/file_system.h"
#include "azer/render/render.h"

class SdkMeshData {
 public:
  SdkMeshData();

  bool LoadFromData(const uint8* data, int32 size);
  bool CreateMesh(std::vector<azer::MeshPtr>* meshes, azer::FileSystem* fs);
 private:
  bool LoadVertexData(const uint8* data, int32 size);
  bool LoadIndicesData(const uint8* data, int32 size);
  bool LoadMaterial(const uint8* data, int32 size);
  bool LoadMesh(const uint8* data, int32 size);

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

  std::vector<Material> mtrls_;
  std::vector<Mesh> meshes_;
  std::vector<azer::SlotVertexDataPtr> vdata_vec_;
  std::vector<azer::IndicesDataPtr> idata_vec_;
  DISALLOW_COPY_AND_ASSIGN(SdkMeshData);
};

