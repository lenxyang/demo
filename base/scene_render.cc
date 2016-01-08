#include "demo/base/scene_render.h"

#include "azer/render/render.h"
#include "lordaeron/env.h"
#include "lordaeron/effect/diffuse_effect.h"
#include "lordaeron/effect/normal_line_effect.h"
#include "lordaeron/interactive/light_controller.h"
#include "lordaeron/scene/render_node.h"
#include "lordaeron/scene/render_env_node.h"
#include "lordaeron/scene/scene_node.h"
#include "lordaeron/scene/scene_renderer.h"
#include "lordaeron/scene/ui_scene_render.h"

using namespace lord;
using namespace azer;
// class ObjectNodeRenderDelegate
ObjectNodeRenderDelegate::ObjectNodeRenderDelegate(
    RenderNode* node, EffectedSceneRenderer* renderer)
    : RenderNodeDelegate(node),
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
      mesh_->AddProvider(node_->GetEnvNode()->delegate());
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
LampNodeRenderDelegate::LampNodeRenderDelegate(RenderNode* node, 
                                               EffectedSceneRenderer* tree_render)
    : RenderNodeDelegate(node),
      tree_render_(tree_render) {
  SceneNode* scene_node = GetSceneNode();
  CHECK(scene_node->type() == kLampSceneNode);
  CHECK(scene_node->parent() && scene_node->parent()->type() == kEnvSceneNode);
  Init();
}

bool LampNodeRenderDelegate::Init() {
  SceneNode* scene_node = GetSceneNode();
  lord::LordEnv* env = lord::LordEnv::instance();
  ResourceLoader* loader = env->resource_loader();
  Light* light = scene_node->mutable_data()->light();
  switch (light->type()) {
    case kDirectionalLight:
      break;
    case kSpotLight: 
      light->InitShadowmapRenderer(gfx::Size(1024, 1024));
      InitShadowMapCamera(light, &camera_);
      scene_renderer_.reset(new ShadowDepthRenderer(loader, light));
      break;
    case kPointLight:
      break;
    default:
      CHECK(false);
  }

  return true;
}

void LampNodeRenderDelegate::Update(const FrameArgs& args) {
  if (scene_renderer_) {
    if (!scene_renderer_->root()) {
      SceneNode* scene_node = GetSceneNode();
      // scene_renderer_->Init(scene_node->root(), &camera_);
      scene_renderer_->Init(scene_node->root(), tree_render_->camera());
    }
    scene_renderer_->Update(args);
  }
}

void LampNodeRenderDelegate::Render(Renderer* orgrenderer) {
  SceneNode* scene_node = GetSceneNode();
  Light* light = scene_node->mutable_data()->light();
  Renderer* renderer = light->shadowmap_renderer();
  if (renderer) {
    renderer->Use();
    renderer->ClearDepthAndStencil();
    renderer->Clear(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    scene_renderer_->Render(renderer);
    orgrenderer->Use();
  }
}

namespace {
class TreeBuildDelegate : public RenderTreeBuilderDelegate {
 public:
  TreeBuildDelegate(EffectedSceneRenderer* renderer)
      : tree_renderer_(renderer) {
  }

  bool NeedRenderNode(SceneNode* node) override {
    if (node->type() == kEnvSceneNode) {
      return false;
    } else {
      return true;
    }
  }
  scoped_ptr<lord::RenderNodeDelegate> CreateRenderDelegate(
      lord::RenderNode* node) override;
  RenderEnvNodeDelegatePtr CreateEnvDelegate(RenderEnvNode* n) override {
    return RenderEnvNodeDelegatePtr(new LordEnvNodeDelegate(n));
  }
 private:
  EffectedSceneRenderer* tree_renderer_;
  DISALLOW_COPY_AND_ASSIGN(TreeBuildDelegate);
};

scoped_ptr<lord::RenderNodeDelegate>
TreeBuildDelegate::CreateRenderDelegate(RenderNode* node) {
  switch (node->GetSceneNode()->type()) {
    case kEnvSceneNode:
      return NULL;
    case kSceneNode:
    case kObjectSceneNode:
      return scoped_ptr<RenderNodeDelegate>(
          new ObjectNodeRenderDelegate(node, tree_renderer_)).Pass();
    case kLampSceneNode:
      return scoped_ptr<RenderNodeDelegate>(
          new LampNodeRenderDelegate(node, tree_renderer_)).Pass();
    default:
      NOTREACHED() << "no such type supported: " << node->GetSceneNode()->type();
      return scoped_ptr<RenderNodeDelegate>().Pass();
  }
}
}

// class EffectedSceneRenderer
EffectedSceneRenderer::EffectedSceneRenderer() {
  scoped_ptr<TreeBuildDelegate> delegate(new TreeBuildDelegate(this));
  SetTreeBuildDelegate(delegate.Pass());
}
