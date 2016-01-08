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
  /*
  SceneNode* scene_node = GetSceneNode();
  lord::LordEnv* env = lord::LordEnv::instance();
  ResourceLoader* loader = env->resource_loader();
  Light* light = scene_node->mutable_data()->light();
  switch (light->type()) {
    case kDirectionalLight:
      break;
    case kSpotLight: 
      InitShadowmapRenderer(gfx::Size(1024, 1024));
      InitShadowMapCamera(light, &camera_);
      scene_renderer_.reset(new ShadowDepthRenderer(loader, light));
      break;
    case kPointLight:
      break;
    default:
      CHECK(false);
  }
  */

  return true;
}

void LampNodeRenderDelegate::Update(const FrameArgs& args) {}
void LampNodeRenderDelegate::Render(Renderer* orgrenderer) {}

// class EffectedEnvNodeDelegate
EffectedEnvNodeDelegate::EffectedEnvNodeDelegate(RenderEnvNode* envnode)
    : RenderEnvNodeDelegate(envnode) {
}

namespace {
RendererPtr InitShadowmapRenderer(const gfx::Size& size) {
  RenderSystem* rs = RenderSystem::Current();
  Texture::Options opt;
  opt.target = (Texture::BindTarget)
      (Texture::kShaderResource | Texture::kRenderTarget);
  opt.format = kRGBAf;
  opt.size = size;
  Viewport viewport(0, 0, opt.size.width(), opt.size.height());
  RendererPtr renderer = rs->CreateRenderer(opt);
  renderer->SetViewport(viewport);
  return renderer;
}
}

void EffectedEnvNodeDelegate::InitLightData(LightData* data) {
  if (data->light->type() == kSpotLight) {
    data->renderer = InitShadowmapRenderer(gfx::Size(512, 512));
    InitShadowMapCamera(data->light, &data->camera);
  }
}

void EffectedEnvNodeDelegate::RebuildLightData(SceneNode* node) {
  light_data_.clear();
  parent_light_data_.clear();
  int32 child_count = node->child_count();
  for (int32 i = 0; i < child_count; ++i) {
    SceneNode* child = node->child_at(i);
    if (child->type() == kLampSceneNode) {
      LightData data;
      data.light = child->mutable_data()->light();
      InitLightData(&data);
      light_data_.push_back(data);
    }
  }

  RenderEnvNode* parent = this->node()->parent();
  if (parent) {
    EffectedEnvNodeDelegate* pdel = 
        static_cast<EffectedEnvNodeDelegate*>(parent->delegate());
    parent_light_data_.assign(pdel->light_data_.begin(), 
                               pdel->light_data_.begin());
    parent_light_data_.assign(pdel->parent_light_data_.begin(), 
                               pdel->parent_light_data_.begin());
  }
}

void EffectedEnvNodeDelegate::Init(SceneNode* scene_node, RenderNode* node) {
  RebuildLightData(scene_node);
}

void EffectedEnvNodeDelegate::RenderDepthMap(LightData* data) {
  if (data->light->type() == kSpotLight) {
  }
}

void EffectedEnvNodeDelegate::OnUpdateNode(const azer::FrameArgs& args) {
  for (uint32 i = 0; i < light_data_.size(); ++i) {
    LightData& data = light_data_[i];
    if (data.light->enable()) {
      RenderDepthMap(&data);
    }
  }
}

int32 EffectedEnvNodeDelegate::light_count() const {
  return light_data_.size() + parent_light_data_.size();
}

const Light* EffectedEnvNodeDelegate::light_at(int32 index) const {
  const LightData* data = light_data_at(index);
  return data->light;
}

const Matrix4& EffectedEnvNodeDelegate::GetLightShadowmapPV(int32 index) const {
  const LightData* data = light_data_at(index);
  return data->camera.GetProjViewMatrix();
}

Texture* EffectedEnvNodeDelegate::GetLightShadowmap(int32 index) const{
  const LightData* data = light_data_at(index);
  return data->renderer->GetRenderTarget(0)->GetTexture();
}

const EffectedEnvNodeDelegate::LightData* EffectedEnvNodeDelegate::light_data_at(
    int32 index) const {
  if (index < light_data_.size()) {
    return &light_data_[index];
  } else if (index <= light_data_.size() + parent_light_data_.size()) {
    return &parent_light_data_[index - light_data_.size()];
  } else {
    CHECK(false);
    return NULL;
  }
}

void EffectedEnvNodeDelegate::UpdateParams(const FrameArgs& args) {}

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
    return RenderEnvNodeDelegatePtr(new EffectedEnvNodeDelegate(n));
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
