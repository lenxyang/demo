#pragma once

#include "base/strings/utf_string_conversions.h"
#include "lordaeron/context.h"
#include "lordaeron/resource/mesh_loader.h"
#include "lordaeron/scene/scene_node.h"
#include "lordaeron/resource/scene_loader.h"

namespace lord {
namespace sandbox {
void InitMeshEffect(azer::Effect* effect, azer::Mesh* mesh);
class SimpleSceneNodeLoader : public SceneNodeLoader {
 public:
  SimpleSceneNodeLoader(azer::FileSystem* fs, azer::Effect* effect)
      : fsystem_(fs), effect_(effect) {}
  virtual const char* node_type_name() const { return "mesh";}
  bool LoadSceneNode(SceneNode* node, azer::ConfigNode* config,
                     SceneLoadContext* lctx) override;
  azer::MeshPtr LoadMesh(azer::ConfigNode* config, SceneLoadContext* lctx);
  azer::EffectParamsProviderPtr LoadProvider(azer::ConfigNode* config,
                                             SceneLoadContext* lctx);
 private:
  azer::FileSystem* fsystem_;
  azer::Effect* effect_;
  DISALLOW_COPY_AND_ASSIGN(SimpleSceneNodeLoader);
};
}  // namespace sandbox
}  // namespace lord
