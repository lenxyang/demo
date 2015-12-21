#include "demo/base/scene_loader.h"

#include "azer/render/render.h"
#include "lordaeron/effect/diffuse_effect.h"
#include "demo/base/effect_dict.h"

using namespace azer;
using namespace lord;

namespace {
void InitMeshEffect(Effect* effect, Mesh* mesh) {
  for (int32 i = 0; i < mesh->part_count(); ++i) {
    mesh->part_at(i)->SetEffect(effect);
  }
}
}  // namespace

MeshNodeLoader::MeshNodeLoader(azer::FileSystem* fs, EffectDict* dict)
    : fsystem_(fs),
      dict_(dict) {
      }

bool MeshNodeLoader::LoadSceneNode(SceneNode* node, ConfigNode* config,
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
  
MeshPtr MeshNodeLoader::LoadMesh(ConfigNode* config, SceneLoadContext* lctx) {
  std::string pathstr;
  if (!config->GetChildText("path", &pathstr)) {
    return MeshPtr();
  }

  std::string effectname;
  if (!config->GetChildText("effect", &effectname)) {
    return MeshPtr();
  }

  Effect* effect = dict_->GetEffect(effectname);
  CHECK(effect) << " cannto find effect named: " << effectname;
  MeshLoader loader(fsystem_);
  ResPath modelpath(::base::UTF8ToUTF16(pathstr));
  MeshPtr obj = loader.Load(modelpath, effect->vertex_desc());
                            
  if (obj.get()) {
    InitMeshEffect(effect, obj.get());
  }
  return obj;
}
