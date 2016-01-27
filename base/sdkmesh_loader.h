#pragma once

#include "base/files/file_path.h"
#include "azer/render/render.h"

struct SdkMesh {
  std::string name;
  std::vector<azer::EntityPtr> entity;
  bool ccw;
  bool pmalpha;
  azer::Vector3 center;
  azer::Vector3 extents;
};

struct SdkModel {
  std::vector<SdkMesh> meshes;
};

bool LoadSDKModel(const ::base::FilePath& path, SdkModel* model);
