#include "demo/base/scene_render.h"

#include "azer/render/render.h"
#include "lordaeron/env.h"
#include "lordaeron/effect/diffuse_effect.h"
#include "lordaeron/effect/normal_line_effect.h"
#include "lordaeron/interactive/light_controller.h"
#include "lordaeron/scene/scene_node.h"
#include "lordaeron/scene/scene_render_tree.h"

using namespace lord;
using namespace azer;
// class ObjectNodeRenderDelegate
ObjectNodeRenderDelegate::ObjectNodeRenderDelegate(
    SceneRenderNode* node, EffectedSceneRenderer* renderer)
    : SceneRenderNodeDelegate(node),
      tree_renderer_(renderer) {
  Init();
}

bool ObjectNodeRenderDelegate::Init() {
  SceneNode* scene_node = GetSceneNode();
  CHECK(scene_node->type() == kObjectSceneNode
        || scene_node->type() == kSceneNode);

  if (scene_node->type() == kObjectSceneNode) {
    mesh_ = scene_node->mutable_data()->GetMesh();
    mesh_->AddProvider(node_);
    if (node_->GetEnvNode())
      mesh_->AddProvider(node_->GetEnvNode());
  }

  return true;
}

void ObjectNodeRenderDelegate::Update(const FrameArgs& args) {
  if (mesh_.get()) {
    mesh_->UpdateProviderParams(args);
  }
}

void ObjectNodeRenderDelegate::Render(Renderer* renderer) {
  if (mesh_.get())
    mesh_->Render(renderer);
}

// class LampNodeRenderDelegate
LampNodeRenderDelegate::LampNodeRenderDelegate(SceneRenderNode* node)
    : SceneRenderNodeDelegate(node) {
  SceneNode* scene_node = GetSceneNode();
  CHECK(scene_node->type() == kLampSceneNode);
  CHECK(scene_node->parent() && scene_node->parent()->type() == kEnvSceneNode);
  Init();
}

bool LampNodeRenderDelegate::Init() {
  SceneNode* scene_node = GetSceneNode();
  Light* light = scene_node->mutable_data()->light();
  switch (light->type()) {
    case kDirectionalLight:
      break;
    case kSpotLight:
      light->InitShadowmapRenderer(gfx::Size(1024, 1024));
      break;
    case kPointLight:
      break;
    default:
      CHECK(false);
  }

  InitShadowMapCamera(light, &camera_);
  lord::LordEnv* env = lord::LordEnv::instance();
  ResourceLoader* loader = env->resource_loader();
  scene_renderer_.reset(new ShadowDepthRenderer(loader, light));
  scene_renderer_->Init(scene_node->root(), &camera_);
  return true;
}

void LampNodeRenderDelegate::Update(const FrameArgs& args) {
  scene_renderer_->Update(args);
}

void LampNodeRenderDelegate::Render(Renderer*) {
  SceneNode* scene_node = GetSceneNode();
  Light* light = scene_node->mutable_data()->light();
  Renderer* renderer = light->shadowmap_renderer();
  if (renderer) {
    scene_renderer_->Render(renderer);
  }
}

namespace {
class NodeDelegateFactory : public lord::SceneRenderNodeDelegateFactory {
 public:
  NodeDelegateFactory(EffectedSceneRenderer* renderer)
      : tree_renderer_(renderer) {}
  scoped_ptr<lord::SceneRenderNodeDelegate> CreateDelegate(
      lord::SceneRenderNode* node) override;
 private:
  EffectedSceneRenderer* tree_renderer_;
};


scoped_ptr<lord::SceneRenderNodeDelegate>
NodeDelegateFactory::CreateDelegate(SceneRenderNode* node) {
  switch (node->GetSceneNode()->type()) {
    case kEnvSceneNode:
      return NULL;
    case kSceneNode:
    case kObjectSceneNode:
      return scoped_ptr<SceneRenderNodeDelegate>(
          new ObjectNodeRenderDelegate(node, tree_renderer_)).Pass();
    case kLampSceneNode:
      return scoped_ptr<SceneRenderNodeDelegate>(
          new LampNodeRenderDelegate(node)).Pass();
    default:
      NOTREACHED() << "no such type supported: " << node->GetSceneNode()->type();
      return scoped_ptr<SceneRenderNodeDelegate>().Pass();
  }
}
}

// class EffectedSceneRenderer
EffectedSceneRenderer::EffectedSceneRenderer() {
}

void EffectedSceneRenderer::Init(lord::SceneNode* root, const Camera* camera) {
  CHECK(root_ == NULL);
  NodeDelegateFactory factory(this);
  SceneRenderTreeBuilder builder(&factory);
  root_ = builder.Build(root, camera);
}

void EffectedSceneRenderer::Update(const FrameArgs& args) {
  UpdateNode(root_, args);
}

void EffectedSceneRenderer::Render(Renderer* renderer) {
  RenderNode(root_, renderer);
}

void EffectedSceneRenderer::UpdateNode(SceneRenderNode* node, const FrameArgs& args) {
  node->Update(args);
  for (auto iter = node->children().begin(); 
       iter != node->children().end(); ++iter) {
    UpdateNode(iter->get(), args);
  }
}

void EffectedSceneRenderer::RenderNode(SceneRenderNode* node, Renderer* renderer) {
  if (!node->GetSceneNode()->visible()) {
    return;
  }

  node->Render(renderer);
  for (auto iter = node->children().begin(); 
       iter != node->children().end(); ++iter) {
    RenderNode(iter->get(), renderer);
  }
}

