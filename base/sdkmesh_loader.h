#pragma once

#include "base/files/file_path.h"
#include "azer/render/render.h"

struct SdkMeshPart {
  azer::EntityPtr entity;
  bool isalpha;
};

struct SdkMesh {
  std::string name;
  std::vector<SdkMeshPart> entity;
  bool ccw;
  bool pmalpha;
  azer::Vector3 center;
  azer::Vector3 extents;
};

struct SdkModel {
  std::vector<SdkMesh> meshes;
};

bool Load(const ::base::FilePath& path, SdkModel* model);
