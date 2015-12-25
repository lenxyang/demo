#include "demo/shadow/shadow_render_tree.h"

#include "azer/render/util/shader_util.h"

using namespace lord;
using namespace azer;

// class ShadowDepthRenderDelegate
const char ShadowDepthRenderDelegate::kEffectProviderName[] = 
    "ShadowDepthRenderDelegate";
ShadowDepthRenderDelegate::ShadowDepthRenderDelegate(
    lord::SceneRenderNode* node, ShadowDepthRenderer* tree_renderer)
    : lord::SceneRenderNodeDelegate(node),
      tree_renderer_(tree_renderer) {
  Init();
}

ShadowDepthRenderDelegate::~ShadowDepthRenderDelegate() {
}

void ShadowDepthRenderDelegate::Init() {
  SceneNode* scene_node = node_->GetSceneNode();
  if (scene_node->type() == kObjectSceneNode) {
    MeshPtr mesh = scene_node->mutable_data()->GetMesh();
    shadow_ = tree_renderer_->CreateShadowMesh(mesh);
    shadow_->AddProvider(this);
  }
}

const azer::Matrix4& ShadowDepthRenderDelegate::GetPV() const {
  return tree_renderer_->GetCamera().GetProjViewMatrix();
}

void ShadowDepthRenderDelegate::UpdateParams(const azer::FrameArgs& args) {
  world_ = node_->GetWorld();
}
  
void ShadowDepthRenderDelegate::Update(const azer::FrameArgs& args) {
  if (shadow_.get())
    shadow_->UpdateProviderParams(args);
}

void ShadowDepthRenderDelegate::Render(azer::Renderer* renderer) {
  if (shadow_.get())
    shadow_->Render(renderer);
}

// class ShadowEffectAdapter
ShadowEffectAdapter::ShadowEffectAdapter() {}
EffectAdapterKey ShadowEffectAdapter::key() const {
  return std::make_pair(typeid(ShadowEffect).name(),
                        typeid(ShadowDepthRenderDelegate).name());
}

void ShadowEffectAdapter::Apply(Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(ShadowEffect));
  CHECK(typeid(*params) == typeid(ShadowDepthRenderDelegate));
  ShadowDepthRenderDelegate* provider = (ShadowDepthRenderDelegate*)params;
  ShadowEffect* effect = dynamic_cast<ShadowEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->GetPV());
}

// class ShadowRenderNodeDelegateFactory
ShadowRenderNodeDelegateFactory::ShadowRenderNodeDelegateFactory(
    ShadowDepthRenderer* renderer)
    : tree_renderer_(renderer) {
}

scoped_ptr<lord::SceneRenderNodeDelegate> ShadowRenderNodeDelegateFactory::
CreateDelegate(lord::SceneRenderNode* node) {
  scoped_ptr<lord::SceneRenderNodeDelegate> p(
      new ShadowDepthRenderDelegate(node, tree_renderer_));
  return p.Pass();
}

// class ShadowDepthRenderer
ShadowDepthRenderer::ShadowDepthRenderer()
    : root_(NULL),
      need_update_(true) {
  effect_ = new ShadowEffect();
}

ShadowDepthRenderer::~ShadowDepthRenderer() {
}

void ShadowDepthRenderer::SetLight(lord::LightPtr light) {
  light_ = light;
  need_update_ = true;
  if (light_->type() == kSpotLight) {
    const Vector3& position = light->spot_light().position;
    const Vector3& dir = light->spot_light().direction;
    camera_.reset(position, position + dir * 10, Vector3(0.0f, 1.0f, 0.0f));
  } else {
    NOTREACHED();
  }
}

void ShadowDepthRenderer::UpdateNode(SceneRenderNode* node,
                                       const azer::FrameArgs& args) {
  node->Update(args);
  for (auto iter = node->children().begin(); 
       iter != node->children().end(); ++iter) {
    UpdateNode(iter->get(), args);
  }
}

void ShadowDepthRenderer::Update(const azer::FrameArgs& args) {
  if (need_update_) {
    UpdateNode(root_, args);
    need_update_ = false;
  }
}

void ShadowDepthRenderer::RenderNode(SceneRenderNode* node, Renderer* renderer) {
  if (!node->GetSceneNode()->visible()) {
    return;
  }

  if (node->GetSceneNode()->type() != kLampSceneNode) {
    node->Render(renderer);
  }

  for (auto iter = node->children().begin(); 
       iter != node->children().end(); ++iter) {
    RenderNode(iter->get(), renderer);
  }
}

void ShadowDepthRenderer::Render(azer::Renderer* renderer) {
  RenderNode(root_, renderer);
}

MeshPartPtr CreateShadowMeshPtr(MeshPart* part, Effect* effect) {
  MeshPartPtr newpart(new MeshPart(effect));
  int32 entity_count = part->entity_count();
  for (int32 i = 0; i < entity_count; ++i) {
    EntityPtr e(new Entity(effect->vertex_desc()));
    e->SetVertexBuffer(part->entity_at(i)->vertex_buffer_at(0), 0);
    e->SetIndicesBuffer(part->entity_at(i)->indices_buffer());
    newpart->AddEntity(e);
  }
  return newpart;
}

MeshPtr ShadowDepthRenderer::CreateShadowMesh(MeshPtr mesh) {
  MeshPtr shadow = new Mesh(mesh->adapter_context());
  for (int32 i = 0; i < mesh->part_count(); ++i) {
    shadow->AddMeshPart(CreateShadowMeshPtr(mesh->part_at(i), effect_));
  }
  return shadow;
}
