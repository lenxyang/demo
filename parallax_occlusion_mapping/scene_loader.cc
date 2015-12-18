#include "demo/parallax_occlusion_mapping/scene_loader.h"

#include "azer/render/render.h"
#include "lordaeron/effect/diffuse_effect.h"
#include "demo/parallax_occlusion_mapping/effect.h"
#include "demo/parallax_occlusion_mapping/effect_adapter.h"

namespace lord {
namespace sandbox {
using namespace azer;
bool SimpleSceneNodeLoader::LoadSceneNode(SceneNode* node, ConfigNode* config,
                                          SceneLoadContext* lctx) {
  Context* ctx = Context::instance(); 
  const std::string& type =  config->GetAttr("type");
  DCHECK(type == "mesh");
  DCHECK(config->HasTaggedChild("mesh"));
  ConfigNode* mesh_node = config->GetTaggedChildren("mesh")[0];
  DCHECK(mesh_node->HasTaggedChild("provider"));
  ConfigNode* provider_node = mesh_node->GetTaggedChildren("provider")[0];
  MeshPtr mesh = LoadMesh(mesh_node, lctx);
  mesh->SetEffectAdapterContext(ctx->GetEffectAdapterContext());
  mesh->AddProvider(LoadProvider(provider_node, lctx));
  node->mutable_data()->AttachMesh(mesh);
  return true;
}
  
MeshPtr SimpleSceneNodeLoader::LoadMesh(ConfigNode* config,
                                        SceneLoadContext* lctx) {
  std::string pathstr;
  if (!config->GetChildText("path", &pathstr)) {
    return MeshPtr();
  }
      
  ModelLoader loader(fsystem_);
  MeshPtr obj = loader.Load(ResPath(::base::UTF8ToUTF16(pathstr)), 
                                  effect_->GetVertexDesc());
  if (obj.get()) {
    InitMeshEffect(effect_, obj.get());
  }
  return obj;
}
  
EffectParamsProviderPtr SimpleSceneNodeLoader::LoadProvider(
    ConfigNode* config, SceneLoadContext* lctx) {
  MaterialProvider* p(new MaterialProvider);
  p->InitFromConfigNode(config, lctx->filesystem);
  return EffectParamsProviderPtr(p);
}

void InitMeshEffect(Effect* effect, Mesh* mesh) {
  for (int32 i = 0; i < mesh->part_count(); ++i) {
    mesh->part_at(i)->SetEffect(effect);
  }
}
}  // namespace sandbox
}  // namespace lord
