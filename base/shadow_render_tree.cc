#include "demo/base/shadow_render_tree.h"

#include "base/strings/utf_string_conversions.h"
#include "azer/render/util/shader_util.h"
#include "lordaeron/resource/resource_util.h"
#include "lordaeron/scene/render_env_node.h"
#include "lordaeron/scene/render_node.h"
#include "lordaeron/scene/scene_renderer.h"
#include "lordaeron/scene/ui_scene_render.h"
#include "demo/base/depth_effect.h"

using namespace lord;
using namespace azer;
using base::UTF8ToUTF16;

// class ShadowDepthRenderDelegate
const char ShadowDepthRenderDelegate::kEffectProviderName[] = 
    "ShadowDepthRenderDelegate";
ShadowDepthRenderDelegate::ShadowDepthRenderDelegate(
    lord::RenderNode* node, ShadowDepthRenderer* tree_renderer)
    : lord::RenderNodeDelegate(node),
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

const Matrix4& ShadowDepthRenderDelegate::GetPV() const {
  return tree_renderer_->camera().GetProjViewMatrix();
}

void ShadowDepthRenderDelegate::UpdateParams(const FrameArgs& args) {
  world_ = node_->GetWorld();
}
  
void ShadowDepthRenderDelegate::Update(const FrameArgs& args) {
  if (shadow_.get())
    shadow_->UpdateProviderParams(args);
}

void ShadowDepthRenderDelegate::Render(Renderer* renderer) {
  if (shadow_.get())
    shadow_->Render(renderer);
}

// class ShadowEffectAdapter
ShadowEffectAdapter::ShadowEffectAdapter() {}
EffectAdapterKey ShadowEffectAdapter::key() const {
  return std::make_pair(typeid(DepthEffect).name(),
                        typeid(ShadowDepthRenderDelegate).name());
}

void ShadowEffectAdapter::Apply(Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(DepthEffect));
  CHECK(typeid(*params) == typeid(ShadowDepthRenderDelegate));
  ShadowDepthRenderDelegate* provider = (ShadowDepthRenderDelegate*)params;
  DepthEffect* effect = dynamic_cast<DepthEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->GetPV());
}

namespace {
class TreeBuildDelegate : public RenderTreeBuilderDelegate {
 public:
  TreeBuildDelegate(ShadowDepthRenderer* renderer);
  bool NeedRenderNode(SceneNode* node) override {
    if (node->type() == kEnvSceneNode) {
      return false;
    } else {
      return true;
    }
  }
  scoped_ptr<lord::RenderNodeDelegate> CreateRenderDelegate(
  lord::RenderNode* node) override {
    scoped_ptr<lord::RenderNodeDelegate> p(
        new ShadowDepthRenderDelegate(node, tree_renderer_));
    return p.Pass();
  }
  RenderEnvNodeDelegatePtr CreateEnvDelegate(RenderEnvNode* n) override {
    return RenderEnvNodeDelegatePtr(new LordEnvNodeDelegate(n));
  }
 private:
  ShadowDepthRenderer* tree_renderer_;
  DISALLOW_COPY_AND_ASSIGN(TreeBuildDelegate);
};
TreeBuildDelegate::TreeBuildDelegate(
    ShadowDepthRenderer* renderer)
    : tree_renderer_(renderer) {
}
}  // namespace

// class ShadowDepthRenderer
ShadowDepthRenderer::ShadowDepthRenderer(ResourceLoader* loader, lord::Light* light)
    : need_update_(true) {
  ResPath effect_path(UTF8ToUTF16("//data/effects.xml:shadow_depth_effect"));
  VariantResource res = LoadResource(effect_path, kResTypeEffect, loader);
  CHECK(res.type == kResTypeEffect);
  effect_ = res.effect;

  SetLight(light);
}

ShadowDepthRenderer::~ShadowDepthRenderer() {
}

void ShadowDepthRenderer::Init(lord::SceneNode* root, const Camera* camera) {
  CHECK(root_ == NULL);
  TreeBuildDelegate delegate(this);
  RenderTreeBuilder builder(&delegate);
  root_ = builder.Build(root, camera);
}

void ShadowDepthRenderer::SetLight(lord::LightPtr light) {
  light_ = light;
  need_update_ = true;
}

void ShadowDepthRenderer::OnUpdateNode(RenderNode* node, const FrameArgs& args) {
  node->Update(args);
  for (auto iter = node->children().begin(); 
       iter != node->children().end(); ++iter) {
    OnUpdateNode(iter->get(), args);
  }
}

void ShadowDepthRenderer::Update(const FrameArgs& args) {
  InitShadowMapCamera(light_, &camera_);
  OnUpdateNode(root_, args);
  need_update_ = false;
}

void ShadowDepthRenderer::OnRenderNode(RenderNode* node, Renderer* renderer) {
  if (!node->GetSceneNode()->visible()) {
    return;
  }

  if (node->GetSceneNode()->type() != kLampSceneNode) {
    node->Render(renderer);
  }

  for (auto iter = node->children().begin(); 
       iter != node->children().end(); ++iter) {
    OnRenderNode(iter->get(), renderer);
  }
}

void ShadowDepthRenderer::Render(Renderer* renderer) {
  OnRenderNode(root_, renderer);
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
