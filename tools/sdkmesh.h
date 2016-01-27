#pragma once

#include <string>
#include <vector>

#include "azer/render/render.h"

class SdkMeshData {
 public:
  bool LoadFromData(const uint8* data, int32 size);
 private:
  struct Material {
    std::string         name;
    bool                per_vertex_color;
    float               specular_power;
    float               alpha;
    azer::Vector3       ambient_color;
    azer::Vector3       diffuse_color;
    azer::Vector3       specular_color;
    azer::Vector3       emissive_color;
    std::string         texture;
  };
  
  struct Part {
    int32 vertex_base;
    int32 start_index;
    int32 vertex_data_index;
    int32 indices_data_index;
    int32 material_index;
    azer::PrimitiveTopology primitive;
  };

  struct Subset {
    std::string name;
    bool ccw;
    azer::Vector3 center;
    azer::Vector3 extends;
    std::vector<Part> part;
  };

  std::vector<Material> mtrls;
  std::vector<Subset> subset;
  std::vector<azer::VertexDataPtr> vdata_;
  std::vector<azer::IndicesDataPtr> idata_;
  DISALLOW_COPY_AND_ASSIGN(SdkMeshData);
};
