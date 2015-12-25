#pragma once

#include "azer/render/render.h"
#include "azer/render/effect_creator.h"
#include "lordaeron/effect/light.h"
#include "lordaeron/scene/scene_node_traverse.h"
#include "lordaeron/scene/scene_render_tree.h"

class ShadowDepthRenderer;
class ShadowDepthRenderDelegate;

class ShadowDepthRenderDelegate : public lord::SceneRenderNodeDelegate,
                                  public azer::EffectParamsProvider {
 public:
  static const char kEffectProviderName[];
  explicit ShadowDepthRenderDelegate(lord::SceneRenderNode* node,
                                     ShadowDepthRenderer* tree_renderer);
  ~ShadowDepthRenderDelegate();
  void Update(const azer::FrameArgs& args) override;
  void Render(azer::Renderer* renderer) override;

  // override from azer::EffectParamsProvider
  const azer::Matrix4& GetWorld() const { return world_;}
  const azer::Matrix4& GetPV() const;
  const char* GetProviderName() const override { return kEffectProviderName;}
  void UpdateParams(const azer::FrameArgs& args) override;
 private:
  void Init();
  ShadowDepthRenderer* tree_renderer_;
  azer::MeshPtr shadow_;
  azer::Matrix4 world_;
  DISALLOW_COPY_AND_ASSIGN(ShadowDepthRenderDelegate);
};

using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;
class ShadowEffectAdapter : public EffectParamsAdapter {
 public:
  ShadowEffectAdapter();
  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(ShadowEffectAdapter);
};


class ShadowRenderNodeDelegateFactory 
    : public lord::SceneRenderNodeDelegateFactory {
 public:
  ShadowRenderNodeDelegateFactory(ShadowDepthRenderer* renderer);
  scoped_ptr<lord::SceneRenderNodeDelegate> CreateDelegate(
      lord::SceneRenderNode* node) override;
 private:
  ShadowDepthRenderer* tree_renderer_;
  DISALLOW_COPY_AND_ASSIGN(ShadowRenderNodeDelegateFactory);
};

class ShadowDepthRenderer : public lord::LightObserver {
 public:
  ShadowDepthRenderer();
  ~ShadowDepthRenderer();
  const azer::Camera& GetCamera() const { return camera_;}
  void SetSceneNode(lord::SceneRenderNode* root) { root_ = root;}
  void SetLight(lord::LightPtr light);
  void Update(const azer::FrameArgs& args);
  void Render(azer::Renderer* renderer);
  azer::MeshPtr CreateShadowMesh(azer::MeshPtr mesh);
 private:
  void UpdateNode(lord::SceneRenderNode* node, const azer::FrameArgs& args);
  void RenderNode(lord::SceneRenderNode* node, azer::Renderer* renderer);
  lord::SceneRenderNode* root_;
  lord::LightPtr light_;
  azer::Camera camera_;
  azer::EffectPtr effect_;
  bool need_update_;
  DISALLOW_COPY_AND_ASSIGN(ShadowDepthRenderer);
};
