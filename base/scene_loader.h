#pragma once

#include "base/strings/utf_string_conversions.h"
#include "lordaeron/context.h"
#include "lordaeron/resource/mesh_loader.h"
#include "lordaeron/scene/scene_node.h"
#include "lordaeron/resource/scene_loader.h"

class EffectDict;
class MeshNodeLoader : public lord::SceneNodeLoader {
 public:
  MeshNodeLoader(azer::FileSystem* fs, EffectDict* dict);

  const char* node_type_name() const override { return "mesh";}
  bool LoadSceneNode(lord::SceneNode* node, azer::ConfigNode* config,
                     lord::SceneLoadContext* lctx) override;
  azer::MeshPtr LoadMesh(azer::ConfigNode* config, lord::SceneLoadContext* lctx);
  azer::EffectParamsProviderPtr LoadProvider(azer::ConfigNode* config,
                                             lord::SceneLoadContext* lctx);
 private:
  azer::FileSystem* fsystem_;
  EffectDict* dict_;
  DISALLOW_COPY_AND_ASSIGN(MeshNodeLoader);
};
