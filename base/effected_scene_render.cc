#include "demo/base/effected_scene_render.h"

#include "azer/render/render.h"
#include "lordaeron/env.h"
#include "lordaeron/effect/diffuse_effect.h"
#include "lordaeron/effect/normal_line_effect.h"
#include "lordaeron/interactive/light_controller.h"
#include "lordaeron/scene/render_node.h"
#include "lordaeron/scene/render_env_node.h"
#include "lordaeron/scene/scene_node.h"
#include "lordaeron/scene/scene_render.h"
#include "lordaeron/scene/ui_scene_render.h"
#include "demo/base/shadow_render_tree.h"

using namespace lord;
using namespace azer;
// class ObjectNodeRenderDelegate
ObjectNodeRenderDelegate::ObjectNodeRenderDelegate(
    RenderNode* node, EffectedSceneRender* renderer)
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
}

void ObjectNodeRenderDelegate::Render(Renderer* renderer) {
  if (mesh_.get())
    mesh_->Render(renderer);
}

// class MirrorObjectNodeRenderDelegate
MirrorObjectNodeRenderDelegate::MirrorObjectNodeRenderDelegate(
    lord::RenderNode* node, EffectedSceneRender* renderer) 
    : ObjectNodeRenderDelegate(node, renderer) {
}

void MirrorObjectNodeRenderDelegate::Update(const azer::FrameArgs& args) {
}

void MirrorObjectNodeRenderDelegate::Render(azer::Renderer* renderer) {
}

// class LampNodeRenderDelegate
LampNodeRenderDelegate::LampNodeRenderDelegate(RenderNode* node, 
                                               EffectedSceneRender* tree_render)
    : RenderNodeDelegate(node),
      tree_render_(tree_render) {
  SceneNode* scene_node = GetSceneNode();
  CHECK(scene_node->type() == kLampSceneNode);
  CHECK(scene_node->parent() && scene_node->parent()->type() == kEnvSceneNode);
  Init();
}

bool LampNodeRenderDelegate::Init() {return true;}
void LampNodeRenderDelegate::Update(const FrameArgs& args) {}
void LampNodeRenderDelegate::Render(Renderer* orgrenderer) {
}

// class EffectedEnvNodeDelegate
EffectedEnvNodeDelegate::EffectedEnvNodeDelegate(RenderEnvNode* envnode,
                                                 EffectedSceneRender* render)
    : RenderEnvNodeDelegate(envnode),
      scene_render_(render),
      args_(NULL) {
  render->AddObserver(this);
  overlay_ = RenderSystem::Current()->CreateOverlay();
  overlay_->SetBounds(gfx::RectF(0.5f, 0.5, 0.5f, 0.5f));
}

EffectedEnvNodeDelegate::~EffectedEnvNodeDelegate() {
  scene_render_->RemoveObserver(this);
  for (auto iter = light_data_.begin(); iter != light_data_.end(); ++iter) {
    delete iter->scene_renderer;
  }
}

namespace {
RendererPtr InitShadowmapRenderer(const gfx::Size& size) {
  RenderSystem* rs = RenderSystem::Current();
  Texture::Options opt;
  opt.target = kBindTargetShaderResource | kBindTargetRenderTarget;
  opt.format = kRGBAf;
  opt.size = size;
  Viewport viewport(0, 0, opt.size.width(), opt.size.height());
  RendererPtr renderer = rs->CreateRenderer(opt);
  renderer->SetViewport(viewport);
  return renderer;
}
}

void EffectedEnvNodeDelegate::InitLightData(LightData* data) {
  data->scene_renderer = NULL;
  if (data->light->type() == kSpotLight) {
    LordEnv* env = LordEnv::instance();
    ResourceLoader* loader = env->resource_loader();
    data->renderer = InitShadowmapRenderer(gfx::Size(1024, 1024));
    InitShadowMapCamera(data->light, &data->camera);
    data->scene_renderer = new ShadowDepthRenderer(loader, data->light);
    data->scene_renderer->Init(data->scene_node->root(), &data->camera);
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
      data.scene_node = node;
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

void EffectedEnvNodeDelegate::RenderDepthMap(LightData* data, Renderer* r) {
  if (data && data->renderer.get()) {
    InitShadowMapCamera(data->light, &data->camera);
    Renderer* renderer = data->renderer;
    RasterizerState* prev_rasterizer_state = renderer->GetRasterizerState();
    DepthStencilState* prev_depth_state = renderer->GetDepthStencilState();
    renderer->Reset();
    renderer->Use();
    renderer->ClearDepthAndStencil();
    renderer->Clear(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    data->scene_renderer->Render(renderer);
    renderer->SetRasterizerState(prev_rasterizer_state);
    renderer->SetDepthStencilState(prev_depth_state, 0);
  }
}

void EffectedEnvNodeDelegate::OnUpdateNode(const FrameArgs& args) {
  args_ = &args;
}

void EffectedEnvNodeDelegate::OnFrameRenderBegin(
    SceneRender* sr, Renderer* renderer) {
  for (uint32 i = 0; i < light_data_.size(); ++i) {
    LightData& data = light_data_[i];
    if (data.light->enable() && data.scene_renderer) {
      data.scene_renderer->Update(*args_);
      InitShadowMapCamera(data.light, &data.camera);
      RenderDepthMap(&data, renderer);
    }
  }

  renderer->Use();
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

namespace {
class TreeBuildDelegate : public RenderTreeBuilderDelegate {
 public:
  TreeBuildDelegate(EffectedSceneRender* renderer)
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
    return RenderEnvNodeDelegatePtr(new EffectedEnvNodeDelegate(n, tree_renderer_));
  }
 private:
  EffectedSceneRender* tree_renderer_;
  DISALLOW_COPY_AND_ASSIGN(TreeBuildDelegate);
};

scoped_ptr<lord::RenderNodeDelegate>
TreeBuildDelegate::CreateRenderDelegate(RenderNode* node) {
  SceneNode* scene_node = node->GetSceneNode();
  switch (node->GetSceneNode()->type()) {
    case kEnvSceneNode:
      return NULL;
    case kSceneNode:
    case kObjectSceneNode:
      if (scene_node->GetAttr("mirror") == "true") {
        return scoped_ptr<RenderNodeDelegate>(
            new MirrorObjectNodeRenderDelegate(node, tree_renderer_)).Pass();
      } else {
        return scoped_ptr<RenderNodeDelegate>(
            new ObjectNodeRenderDelegate(node, tree_renderer_)).Pass();
      }
    case kLampSceneNode:
      return scoped_ptr<RenderNodeDelegate>(
          new LampNodeRenderDelegate(node, tree_renderer_)).Pass();
    default:
      NOTREACHED() << "no such type supported: " << node->GetSceneNode()->type();
      return scoped_ptr<RenderNodeDelegate>().Pass();
  }
}
}

// class EffectedSceneRender
EffectedSceneRender::EffectedSceneRender() {
  scoped_ptr<TreeBuildDelegate> delegate(new TreeBuildDelegate(this));
  SetTreeBuildDelegate(delegate.Pass());
}
